// Bounded private C ABI. No image IO, Python bindings or linear optimizer.
// Arrays are x-fastest; geometry and returned displacement are physical RAS mm.
#include <algorithm>
#include <cmath>
#include <cstring>
#include <functional>
#include <itkANTSNeighborhoodCorrelationImageToImageMetricv4.h>
#include <itkAffineTransform.h>
#include <itkBSplineControlPointImageFilter.h>
#include <itkBSplineTransform.h>
#include <itkBSplineTransformInitializer.h>
#include <itkCommand.h>
#include <itkConstantPadImageFilter.h>
#include <itkGradientDescentLineSearchOptimizerv4.h>
#include <itkImage.h>
#include <itkImageMaskSpatialObject.h>
#include <itkImageRegistrationMethodv4.h>
#include <itkMattesMutualInformationImageToImageMetricv4.h>
#include <itkN4BiasFieldCorrectionImageFilter.h>
#include <itkOtsuThresholdImageFilter.h>
#include <itkRegistrationParameterScalesFromPhysicalShift.h>
#include <itkShrinkImageFilter.h>
#include <itkVersion.h>
#include <nlohmann/json.hpp>
#include <string>
#ifdef _WIN32
#define API __declspec(dllexport)
#else
#define API __attribute__((visibility("default")))
#endif
using J = nlohmann::json;
using Image = itk::Image<float, 3>;
using Mask = itk::Image<unsigned char, 3>;
using Affine = itk::AffineTransform<double, 3>;
using Spline = itk::BSplineTransform<double, 3, 3>;
using Optimizer = itk::GradientDescentLineSearchOptimizerv4;
using Registration = itk::ImageRegistrationMethodv4<Image, Image, Spline>;
using Callback = int (*)(const char *, void *);
struct Notify : itk::Command {
  using Self = Notify;
  using Pointer = itk::SmartPointer<Self>;
  itkNewMacro(Self);
  std::function<void()> action;
  void Execute(itk::Object *, const itk::EventObject &) override { action(); }
  void Execute(const itk::Object *, const itk::EventObject &) override {
    action();
  }
};
static void store(const J &j, char *out, size_t capacity) {
  auto s = j.dump();
  if (s.size() + 1 > capacity)
    throw std::runtime_error("Native metadata buffer exceeded");
  std::memcpy(out, s.c_str(), s.size() + 1);
}
static bool event(Callback cb, void *context, const J &data) {
  return cb && cb(data.dump().c_str(), context) != 0;
}
static size_t count(const int32_t *d) {
  size_t n = 1;
  for (int i = 0; i < 3; i++) {
    if (d[i] < 8 || d[i] > 2000000)
      throw std::runtime_error("Invalid image dimensions");
    n *= size_t(d[i]);
    if (n > 2000000)
      throw std::runtime_error("Image exceeds 2 million voxels");
  }
  return n;
}
template <class T>
typename T::Pointer image(const typename T::PixelType *data, const int32_t *d,
                          const double *a) {
  count(d);
  typename T::SizeType size;
  typename T::PointType origin;
  typename T::SpacingType spacing;
  typename T::DirectionType direction;
  for (int c = 0; c < 3; c++) {
    size[c] = d[c];
    spacing[c] =
        std::sqrt(a[c] * a[c] + a[4 + c] * a[4 + c] + a[8 + c] * a[8 + c]);
    if (!std::isfinite(spacing[c]) || spacing[c] < 1e-5)
      throw std::runtime_error("Invalid image geometry");
    origin[c] = (c < 2 ? -1 : 1) * a[4 * c + 3];
    for (int r = 0; r < 3; r++)
      direction[r][c] = (r < 2 ? -1 : 1) * a[4 * r + c] / spacing[c];
  }
  for (int i = 0; i < 3; i++)
    for (int j = 0; j < 3; j++) {
      double dot = 0;
      for (int r = 0; r < 3; r++)
        dot += direction[r][i] * direction[r][j];
      if (std::abs(dot - (i == j ? 1 : 0)) > 1e-5)
        throw std::runtime_error(
            "Sheared grids require explicit orthogonal resampling");
    }
  auto result = T::New();
  result->SetRegions(size);
  result->SetOrigin(origin);
  result->SetSpacing(spacing);
  result->SetDirection(direction);
  result->Allocate();
  std::copy(data, data + count(d), result->GetBufferPointer());
  return result;
}
extern "C" API const char *align_itk_version() {
  return itk::Version::GetITKVersion();
}
extern "C" API int
align_itk_bspline(const float *f, const int32_t *fd, const double *fa,
                  const float *m, const int32_t *md, const double *ma,
                  const unsigned char *fm, const unsigned char *mm,
                  const double *pull, const char *settings, float *residual,
                  char *output, size_t capacity, Callback cb, void *context) {
  try {
    const auto s = J::parse(settings);
    const auto threads = s.at("threads").get<unsigned>();
    itk::MultiThreaderBase::SetGlobalDefaultNumberOfThreads(threads);
    auto fixed = image<Image>(f, fd, fa), moving = image<Image>(m, md, ma);
    auto base = Affine::New();
    Affine::MatrixType matrix;
    Affine::OutputVectorType translation;
    for (int r = 0; r < 3; r++) {
      translation[r] = (r < 2 ? -1 : 1) * pull[4 * r + 3];
      for (int c = 0; c < 3; c++)
        matrix[r][c] = (r < 2 ? -1 : 1) * (c < 2 ? -1 : 1) * pull[4 * r + c];
    }
    base->SetMatrix(matrix);
    base->SetTranslation(translation);
    auto spline = Spline::New();
    auto initializer = itk::BSplineTransformInitializer<Spline, Image>::New();
    Spline::MeshSizeType mesh;
    for (int c = 0; c < 3; c++) {
      mesh[c] = std::max(
          1., std::nearbyint((fd[c] - 1) * fixed->GetSpacing()[c] /
                             s.at("bspline_spacing_mm").get<double>()));
      if (mesh[c] > 16)
        throw std::runtime_error("B-spline mesh exceeds 16 cells per axis");
    }
    initializer->SetTransform(spline);
    initializer->SetImage(fixed);
    initializer->SetTransformDomainMeshSize(mesh);
    initializer->InitializeTransform();
    spline->SetIdentity();
    auto registration = Registration::New();
    registration->SetFixedImage(fixed);
    registration->SetMovingImage(moving);
    registration->SetMovingInitialTransform(base);
    registration->SetInitialTransform(spline);
    registration->InPlaceOn();
    using Metric = itk::ImageToImageMetricv4<Image, Image>;
    Metric::Pointer metric;
    if (s.at("metric") == "mattes-mi") {
      auto mi =
          itk::MattesMutualInformationImageToImageMetricv4<Image, Image>::New();
      mi->SetNumberOfHistogramBins(s.at("bins").get<unsigned>());
      metric = mi;
    } else {
      using NC =
          itk::ANTSNeighborhoodCorrelationImageToImageMetricv4<Image, Image>;
      auto nc = NC::New();
      NC::RadiusType radius;
      radius.Fill(s.at("local_radius").get<unsigned>());
      nc->SetRadius(radius);
      metric = nc;
    }
    metric->SetMaximumNumberOfWorkUnits(threads);
    using MaskObject = itk::ImageMaskSpatialObject<3>;
    if (fm) {
      auto object = MaskObject::New();
      object->SetImage(image<Mask>(fm, fd, fa));
      metric->SetFixedImageMask(object);
    }
    if (mm) {
      auto object = MaskObject::New();
      object->SetImage(image<Mask>(mm, md, ma));
      metric->SetMovingImageMask(object);
    }
    registration->SetMetric(metric);
    auto factors = s.at("shrink_factors").get<std::vector<unsigned>>();
    auto sigmas = s.at("smoothing_sigmas_mm").get<std::vector<double>>();
    registration->SetNumberOfLevels(factors.size());
    Registration::ShrinkFactorsArrayType shrink(factors.size());
    Registration::SmoothingSigmasArrayType smoothing(factors.size());
    for (size_t i = 0; i < factors.size(); i++) {
      shrink[i] = factors[i];
      smoothing[i] = sigmas[i];
    }
    registration->SetShrinkFactorsPerLevel(shrink);
    registration->SetSmoothingSigmasPerLevel(smoothing);
    registration->SmoothingSigmasAreSpecifiedInPhysicalUnitsOn();
    if (s.at("sampling_fraction").get<double>() < 1) {
      registration->SetMetricSamplingStrategy(
          Registration::MetricSamplingStrategyEnum::RANDOM);
      registration->SetMetricSamplingPercentage(
          s.at("sampling_fraction").get<double>());
      registration->MetricSamplingReinitializeSeed(
          s.at("seed").get<unsigned>());
    } else
      registration->SetMetricSamplingStrategy(
          Registration::MetricSamplingStrategyEnum::NONE);
    auto optimizer = Optimizer::New();
    optimizer->SetLearningRate(s.at("learning_rate").get<double>());
    optimizer->SetNumberOfIterations(
        s.at("nonlinear_iterations").get<unsigned>());
    optimizer->SetMinimumConvergenceValue(1e-6);
    optimizer->SetConvergenceWindowSize(10);
    optimizer->SetUpperLimit(5.);
    optimizer->SetLowerLimit(0.);
    optimizer->SetEpsilon(.01);
    optimizer->SetMaximumLineSearchIterations(10);
    optimizer->SetMaximumStepSizeInPhysicalUnits(
        s.at("maximum_step_mm").get<double>());
    optimizer->SetDoEstimateLearningRateOnce(true);
    optimizer->SetDoEstimateLearningRateAtEachIteration(false);
    using Scales = itk::RegistrationParameterScalesFromPhysicalShift<Metric>;
    auto scales = Scales::New();
    scales->SetMetric(metric);
    scales->SetTransformForward(true);
    optimizer->SetScalesEstimator(scales);
    registration->SetOptimizer(optimizer);
    bool cancelled = false;
    J levels = J::array();
    auto observer = Notify::New();
    observer->action = [&]() {
      if (event(cb, context,
                {{"stage", "itk-nonlinear"},
                 {"level", registration->GetCurrentLevel()},
                 {"iteration", optimizer->GetCurrentIteration()},
                 {"message", "ITK nonlinear registration"}})) {
        cancelled = true;
        optimizer->StopOptimization();
      }
    };
    optimizer->AddObserver(itk::IterationEvent(), observer);
    auto level = Notify::New();
    level->action = [&]() {
      observer->action();
      auto l = registration->GetCurrentLevel();
      levels.push_back({{"level", l},
                        {"shrink_factor", factors[l]},
                        {"sigma_mm", sigmas[l]}});
    };
    registration->AddObserver(itk::MultiResolutionIterationEvent(), level);
    if (event(cb, context, {{"stage", "itk-nonlinear"}}))
      throw std::runtime_error("Cancelled");
    registration->Update();
    if (cancelled)
      throw std::runtime_error("Cancelled");
    // K(y)-y is the fixed-space spline before the unchanged accepted affine
    // pull.
    auto n = count(fd);
    for (size_t i = 0; i < n; i++) {
      if (i % size_t(fd[0] * fd[1]) == 0 &&
          event(cb, context, {{"stage", "itk-field"}}))
        throw std::runtime_error("Cancelled");
      Image::IndexType index;
      index[0] = i % fd[0];
      index[1] = (i / fd[0]) % fd[1];
      index[2] = i / (fd[0] * fd[1]);
      Image::PointType p;
      fixed->TransformIndexToPhysicalPoint(index, p);
      auto q = spline->TransformPoint(p);
      for (int c = 0; c < 3; c++)
        residual[i + c * n] = float((c < 2 ? -1 : 1) * (q[c] - p[c]));
    }
    J origin = J::array(), dimensions = J::array(), direction = J::array(),
      meshout = J::array(), spacing = J::array();
    for (int r = 0; r < 3; r++) {
      origin.push_back(spline->GetTransformDomainOrigin()[r]);
      dimensions.push_back(spline->GetTransformDomainPhysicalDimensions()[r]);
      meshout.push_back(mesh[r]);
      spacing.push_back(spline->GetTransformDomainPhysicalDimensions()[r] /
                        mesh[r]);
      for (int c = 0; c < 3; c++)
        direction.push_back(spline->GetTransformDomainDirection()[r][c]);
    }
    store({{"library_version", itk::Version::GetITKVersion()},
           {"metric_value", optimizer->GetValue()},
           {"valid_metric_samples", metric->GetNumberOfValidPoints()},
           {"iterations_final_level", optimizer->GetCurrentIteration()},
           {"stop_condition", optimizer->GetStopConditionDescription()},
           {"levels", levels},
           {"bspline_mesh_size", meshout},
           {"bspline_domain",
            {{"order", 3},
             {"origin_LPS", origin},
             {"physical_dimensions_mm", dimensions},
             {"direction_LPS", direction},
             {"mesh_size", meshout},
             {"effective_cell_spacing_mm", spacing},
             {"parameter_count", spline->GetNumberOfParameters()}}}},
          output, capacity);
    return 0;
  } catch (const std::exception &e) {
    try {
      store({{"error", e.what()}}, output, capacity);
    } catch (...) {
    }
    return 1;
  }
}
extern "C" API int
align_itk_n4(const float *values, const int32_t *dims, const double *affine,
             const unsigned char *supplied, const char *settings,
             float *corrected, float *bias, unsigned char *selected,
             char *output, size_t capacity, Callback cb, void *context) {
  try {
    const auto s = J::parse(settings);
    const auto n = count(dims);
    itk::MultiThreaderBase::SetGlobalDefaultNumberOfThreads(
        s.at("threads").get<unsigned>());
    auto input = image<Image>(values, dims, affine);
    Mask::Pointer mask;
    if (supplied)
      mask = image<Mask>(supplied, dims, affine);
    else {
      auto otsu = itk::OtsuThresholdImageFilter<Image, Mask>::New();
      otsu->SetInput(input);
      otsu->SetInsideValue(0);
      otsu->SetOutsideValue(1);
      otsu->SetNumberOfHistogramBins(s.at("histogram_bins").get<unsigned>());
      otsu->Update();
      mask = otsu->GetOutput();
      mask->DisconnectPipeline();
      for (size_t i = 0; i < n; i++)
        mask->GetBufferPointer()[i] =
            mask->GetBufferPointer()[i] && values[i] > 0;
    }
    size_t mask_count = 0;
    float minimum = INFINITY, maximum = -INFINITY;
    for (size_t i = 0; i < n; i++) {
      selected[i] = mask->GetBufferPointer()[i];
      if (selected[i]) {
        if (!(values[i] > 0))
          throw std::runtime_error("Masked intensities must be positive");
        mask_count++;
        minimum = std::min(minimum, values[i]);
        maximum = std::max(maximum, values[i]);
      }
    }
    if (mask_count < 64 ||
        maximum - minimum <= std::numeric_limits<float>::epsilon())
      throw std::runtime_error(
          "Too few positive varying foreground voxels for N4");
    using Shrink = itk::ShrinkImageFilter<Image, Image>;
    using MaskShrink = itk::ShrinkImageFilter<Mask, Mask>;
    auto shrink = Shrink::New();
    auto mask_shrink = MaskShrink::New();
    shrink->SetInput(input);
    mask_shrink->SetInput(mask);
    J factors = J::array();
    for (unsigned c = 0; c < 3; c++) {
      unsigned f =
          std::max(1., std::floor(s.at("estimation_spacing_mm").get<double>() /
                                      input->GetSpacing()[c] +
                                  1e-6));
      f = std::min(f, unsigned(std::max(1, (dims[c] - 1) / 7)));
      shrink->SetShrinkFactor(c, f);
      mask_shrink->SetShrinkFactor(c, f);
      factors.push_back(f);
    }
    shrink->Update();
    mask_shrink->Update();
    auto fit = shrink->GetOutput();
    auto fit_mask = mask_shrink->GetOutput();
    size_t usable = 0;
    for (size_t i = 0;
         i < fit_mask->GetLargestPossibleRegion().GetNumberOfPixels(); i++)
      usable += fit_mask->GetBufferPointer()[i] != 0;
    if (usable < 64)
      throw std::runtime_error("Too few foreground samples after estimation "
                               "downsampling; lower estimation spacing");
    Image::SizeType lower, upper;
    J padded = J::array(), points = J::array(), actual_spacing = J::array(),
      initial_spacing = J::array(), finest_spacing = J::array();
    size_t padded_count = 1, control_count = 1;
    auto iterations = s.at("iterations").get<std::vector<unsigned>>();
    using N4 = itk::N4BiasFieldCorrectionImageFilter<Image, Mask, Image>;
    N4::ArrayType control;
    for (unsigned c = 0; c < 3; c++) {
      double spacing = fit->GetSpacing()[c],
             domain =
                 (fit->GetLargestPossibleRegion().GetSize()[c] - 1) * spacing;
      unsigned mesh = std::max(
          1.,
          std::ceil(domain / s.at("initial_spline_spacing_mm").get<double>()));
      unsigned extra = std::max(
          0., std::ceil(mesh * s.at("initial_spline_spacing_mm").get<double>() /
                            spacing -
                        (fit->GetLargestPossibleRegion().GetSize()[c] - 1)));
      lower[c] = extra / 2;
      upper[c] = extra - lower[c];
      size_t size = fit->GetLargestPossibleRegion().GetSize()[c] + extra;
      padded_count *= size;
      control_count *= mesh * (1u << (iterations.size() - 1)) + 3;
      control[c] = mesh + 3;
      padded.push_back(size);
      points.push_back(control[c]);
      actual_spacing.push_back(spacing);
      initial_spacing.push_back((size - 1) * spacing / mesh);
      finest_spacing.push_back((size - 1) * spacing / mesh /
                               (1u << (iterations.size() - 1)));
    }
    if (padded_count > 16000000 || control_count > 1000000)
      throw std::runtime_error(
          "N4 control or estimation grid too large; adjust spacing or levels");
    using Pad = itk::ConstantPadImageFilter<Image, Image>;
    using MaskPad = itk::ConstantPadImageFilter<Mask, Mask>;
    auto pad = Pad::New();
    pad->SetInput(fit);
    pad->SetPadLowerBound(lower);
    pad->SetPadUpperBound(upper);
    pad->SetConstant(0);
    auto mask_pad = MaskPad::New();
    mask_pad->SetInput(fit_mask);
    mask_pad->SetPadLowerBound(lower);
    mask_pad->SetPadUpperBound(upper);
    mask_pad->SetConstant(0);
    auto n4 = N4::New();
    n4->SetInput(pad->GetOutput());
    n4->SetMaskImage(mask_pad->GetOutput());
    n4->SetMaskLabel(1);
    n4->SetUseMaskLabel(true);
    n4->SetNumberOfFittingLevels(iterations.size());
    N4::VariableSizeArrayType caps(iterations.size());
    for (size_t i = 0; i < iterations.size(); i++)
      caps[i] = iterations[i];
    n4->SetMaximumNumberOfIterations(caps);
    n4->SetNumberOfControlPoints(control);
    n4->SetSplineOrder(3);
    n4->SetConvergenceThreshold(s.at("convergence").get<double>());
    n4->SetNumberOfHistogramBins(s.at("histogram_bins").get<unsigned>());
    n4->SetBiasFieldFullWidthAtHalfMaximum(
        s.at("bias_field_fwhm").get<double>());
    n4->SetWienerFilterNoise(s.at("wiener_noise").get<double>());
    J finals = J::object();
    bool cancelled = false;
    auto observer = Notify::New();
    observer->action = [&]() {
      unsigned l = n4->GetCurrentLevel();
      J e = {{"level", l},
             {"iteration", n4->GetElapsedIterations()},
             {"convergence", n4->GetCurrentConvergenceMeasurement()},
             {"message", "N4 bias correction"}};
      finals[std::to_string(l)] = e;
      if (event(cb, context, e)) {
        cancelled = true;
        n4->AbortGenerateDataOn();
      }
    };
    n4->AddObserver(itk::IterationEvent(), observer);
    if (event(cb, context, {{"stage", "n4"}}))
      throw std::runtime_error("Cancelled");
    n4->Update();
    if (cancelled)
      throw std::runtime_error("Cancelled");
    using Reconstruct = itk::BSplineControlPointImageFilter<
        N4::BiasFieldControlPointLatticeType, N4::ScalarImageType>;
    auto reconstruct = Reconstruct::New();
    reconstruct->SetInput(n4->GetLogBiasFieldControlPointLattice());
    reconstruct->SetSplineOrder(3);
    reconstruct->SetSize(input->GetLargestPossibleRegion().GetSize());
    reconstruct->SetOrigin(input->GetOrigin());
    reconstruct->SetSpacing(input->GetSpacing());
    reconstruct->SetDirection(input->GetDirection());
    reconstruct->Update();
    auto field = reconstruct->GetOutput();
    float low = INFINITY, high = 0;
    for (size_t i = 0; i < n; i++) {
      if (i % size_t(dims[0] * dims[1]) == 0 &&
          event(cb, context, {{"stage", "n4-field"}}))
        throw std::runtime_error("Cancelled");
      bias[i] = std::exp(field->GetBufferPointer()[i][0]);
      corrected[i] = values[i] / bias[i];
      if (!std::isfinite(bias[i]) || bias[i] <= 0 ||
          !std::isfinite(corrected[i]))
        throw std::runtime_error("N4 produced unstable values");
      low = std::min(low, bias[i]);
      high = std::max(high, bias[i]);
    }
    J warnings = J::array();
    if (minimum < 1)
      warnings.push_back("Some fitted intensities are below 1; no automatic "
                         "offset was applied.");
    if (s.at("initial_spline_spacing_mm").get<double>() /
            (1u << (iterations.size() - 1)) <
        5)
      warnings.push_back("The finest bias-field scale is below 5 mm; it may "
                         "fit anatomy rather than smooth shading.");
    for (auto it = finals.begin(); it != finals.end(); ++it) {
      auto l = std::stoul(it.key());
      if (it.value()["iteration"].get<unsigned>() >= iterations[l] &&
          it.value()["convergence"].get<double>() >
              s.at("convergence").get<double>())
        warnings.push_back("A fitting level reached its iteration cap; inspect "
                           "correction quality.");
    }
    store({{"version", itk::Version::GetITKVersion()},
           {"mask_method", supplied ? "User-provided binary estimation mask"
                                    : "Otsu foreground intersected with "
                                      "strictly positive intensities"},
           {"mask_voxels", mask_count},
           {"shrink_factors", factors},
           {"actual_estimation_spacing_mm", actual_spacing},
           {"padded_estimation_shape", padded},
           {"control_points", points},
           {"actual_initial_spline_spacing_mm", initial_spacing},
           {"actual_finest_spline_spacing_mm", finest_spacing},
           {"bias_range", {low, high}},
           {"final_levels", finals},
           {"warnings", warnings}},
          output, capacity);
    return 0;
  } catch (const std::exception &e) {
    try {
      store({{"error", e.what()}}, output, capacity);
    } catch (...) {
    }
    return 1;
  }
}
