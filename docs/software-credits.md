# Software credits and references

Brainana Align combines its own application and registration code with the projects
below. Credit belongs to their authors and contributors. The versions listed here
identify the incorporated source or pinned dependency; `package-lock.json`,
`scientific/requirements.txt`, `scientific/native-itk/UPSTREAM.json` and the release
manifests provide the build-specific record.

In a full release bundle, extract the matching source ZIP to follow local
source/license links; the same links work directly in the GitHub repository.

## Imaging and numerical software

| Project | Use in Align | Source, acknowledgement and license record |
|---|---|---|
| **JIP / Joe's Image Programs**, developed by **Joseph B. Mandeville**, Massachusetts General Hospital / Athinoula A. Martinos Center | Repaired and adapted native nonlinear registration engine. Align supplies its accepted linear transform and explicitly converts between JIP's centered coordinates and image/world coordinates. | [Original project documentation](https://www.nmr.mgh.harvard.edu/~jbm/jip/), [incorporated source](../scientific/native/source/), [original MGH software license](../scientific/native/source/engine/JIP-LICENSE.txt), [modification record](jip-changes.md). Internal Align engine/worker version numbers are not upstream JIP release numbers. |
| **Insight Toolkit (ITK) 5.4.6**, Insight Software Consortium and contributors | Native cubic B-spline nonlinear registration and N4 bias correction, statically linked into Align's small native adapter. | [ITK](https://itk.org/), [pinned source/hash and scope](../scientific/native-itk/UPSTREAM.json), [adapter](../scientific/native-itk/align_itk.cpp), [Apache-2.0 license](../scientific/native-itk/licenses/ITK-LICENSE) and [notices](../scientific/native-itk/licenses/ITK-NOTICE). |
| **NiiVue 0.69.0**, NiiVue authors and contributors | WebGL2 image viewing and rendering inside the desktop application. Align adds its own navigation, crosshair, overlay and comparison controls around the viewer. | [Project](https://niivue.com/), [source and BSD-2-Clause license](https://github.com/niivue/niivue). |
| **NumPy 1.26.4**, NumPy developers | Numerical arrays, geometry calculations and data exchange with the native worker. Intel macOS builds use this same version with system Accelerate to reduce bundle size. | [Project](https://numpy.org/), [recommended citation](https://numpy.org/citing-numpy/), [source/license](https://github.com/numpy/numpy/tree/v1.26.4). |
| **NiBabel 5.3.2**, NiBabel developers | NIfTI image and header handling in the scientific worker. | [Project and documentation](https://nipy.org/nibabel/), [source, license and release citation instructions](https://github.com/nipy/nibabel). Preserve its MIT/BSD notices; use the citation for the release actually used. |
| **FFTW 3.2.2**, Matteo Frigo, Steven G. Johnson and contributors | FFT routines used by the incorporated JIP code. | [Project](https://www.fftw.org/), [vendored source](../scientific/native/source/fftw-3.2.2/), [COPYING](../scientific/native/source/fftw-3.2.2/COPYING). Retain the separate notices in individual files. |
| **NIfTI C library / znzlib**, NIfTI DFWG and contributors | Native image format and spatial-matrix support inherited through JIP. | [Vendored source](../scientific/native/source/nifticlib-2.0.0/), [license](../scientific/native/source/nifticlib-2.0.0/LICENSE), [NIfTI reference implementation](https://github.com/NIFTI-Imaging/nifti_clib). |
| **zlib 1.3.1**, Jean-loup Gailly, Mark Adler and contributors | Native compression support. | [Project](https://zlib.net/), [vendored source and notices](../scientific/native/source/zlib-1.3.1/). |
| **lmfit / MINPACK lineage**, Burton S. Garbow, Kenneth E. Hillstrom, Jorge J. More, Steve Moshier and Joachim Wuttke | Fitting support retained in JIP's native source. | [Incorporated implementation and author notices](../scientific/native/source/lib/lmmin/lmmin.c), [public interface/license notice](../scientific/native/source/lib/lmmin/lmmin.h). |
| **nlohmann/json 3.12.0**, Niels Lohmann and contributors | JSON configuration and results in the native ITK adapter. | [Project](https://github.com/nlohmann/json), [retained MIT license](../scientific/native-itk/vendor/NLOHMANN-LICENSE.MIT). |

## Desktop, data access and build tools

- **Electron / Chromium / Node.js** provide the dedicated desktop application and
  its local services. See [Electron](https://www.electronjs.org/),
  [Node.js](https://nodejs.org/) and their bundled license/notices. Electron is
  pinned to 43.5.1; the exact embedded Node version is recorded by package validation.
- **Python** runs the thin numerical adapter inside the bundled scientific worker;
  users do not install Python separately. Retained notices are in
  [scientific/native/licenses](../scientific/native/licenses/).
- **PyInstaller** freezes that worker. Its GPL license includes a bootloader
  exception; retain the [distributed license text](../scientific/native/licenses/PyInstaller-COPYING.txt).
- **nifti-reader-js**, **fflate** and **ssh2** provide frontend NIfTI parsing,
  compression and SSH/SFTP support. Sources:
  [NIFTI-Reader-JS](https://github.com/rii-mango/NIFTI-Reader-JS),
  [fflate](https://github.com/101arrowz/fflate),
  [ssh2](https://github.com/mscdex/ssh2). Resolved versions are in the lockfile.
- **Brainana Viewer** supplied the source/release organization reference and
  adapted SFTP/known-host verification code and fixtures. See
  [Viewer](https://github.com/brainana/brainana-viewer) and [NOTICE.txt](../NOTICE.txt)
  for the recorded provenance. Align retains its separate registration workflow.
- **Vite**, **TypeScript**, **CMake**, **Ninja**, **Clang/GCC**, **MSYS2/MinGW** and
  GitHub Actions build the application. The Intel NumPy source-build tools are
  pinned separately in [requirements-macos-intel.txt](../scripts/ci/requirements-macos-intel.txt).
- **SimpleITK 2.5.3** is an independent displacement-file reader used in validation
  only. It is excluded from the shipped worker. See [SimpleITK](https://simpleitk.org/)
  and [the independent reader regression](../tests/displacement-simpleitk-regression.mts).

AFNI, FSL, ANTs and elastix references in Align's **linear defaults** profiles
identify settings and methods used for comparison. Their registration executables
are not incorporated as separate linear engines, and the profiles do not promise
identical results. The reviewed sources are listed in the
[profile reference catalogue](../Documentation/research/LINEAR_DEFAULTS_REFERENCES.json).

## Scientific references

When reporting work performed with Align, record the Align release and the actual
registration/preprocessing methods used. Credit the relevant upstream projects;
using one method does not imply every method below was used.

1. **JIP:** Joseph B. Mandeville, *JIP Analysis Toolkit / Joe's Image Programs*,
   Massachusetts General Hospital / Athinoula A. Martinos Center.
   [Project documentation](https://www.nmr.mgh.harvard.edu/~jbm/jip/).
   Acknowledge the Hospital as specified in the retained JIP license and identify
   this distribution as the modified implementation incorporated in Brainana Align.
2. **N4 bias correction:** Tustison NJ, Avants BB, Cook PA, Zheng Y, Egan A,
   Yushkevich PA, Gee JC. *N4ITK: Improved N3 Bias Correction.* IEEE Transactions
   on Medical Imaging. 2010;29(6):1310–1320.
   [doi:10.1109/TMI.2010.2046908](https://doi.org/10.1109/TMI.2010.2046908).
3. **NumPy:** Harris CR et al. *Array programming with NumPy.* Nature.
   2020;585:357–362.
   [doi:10.1038/s41586-020-2649-2](https://doi.org/10.1038/s41586-020-2649-2).
4. **FFTW:** Frigo M, Johnson SG. *The Design and Implementation of FFTW3.*
   Proceedings of the IEEE. 2005;93(2):216–231.
   [Authors' paper](https://www.fftw.org/fftw-paper-ieee.pdf).

For ITK, NiiVue and NiBabel, also follow their upstream software/version citation
instructions. Original component licenses remain authoritative: Align's
[AGPL license](../LICENSE) does not replace third-party copyrights or terms.
