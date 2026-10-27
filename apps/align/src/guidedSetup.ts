import {renderGuideText} from './guideText'
import {projectGuidePriority,isGuidePrerequisite} from './operationState'
import {imagingText} from './registrationPairing'
import {proposalDecisionControl} from './proposalDecision'
import {guideNavigationStep} from './guideTeaching'
import {hasLandmarkBasis,isLocalRegistration} from './registrationWorkflow'
import {placeGuide,clampGuidePosition} from './guidePlacement'
import {getPreferences,preferenceStorage} from './preferences'
import type { FitState, Landmark } from './appTypes'
export type GuideRoute = 'choose' | 'registration' | 'processing' | 'processing-mask' | 'processing-crop' | 'processing-voxels' | 'inspect' | 'landmarks' | 'automatic' | 'manual' | 'review' | 'windows' | 'refinement' | 'spline' | 'export' | 'done'
export type GuideState = { welcome?:boolean; preparation?:{mri:boolean;ct:boolean;ready:boolean;open?:boolean}; mri: boolean; ct: boolean; landmarks: Landmark[]; selectedId: number | null; minimumPairs: number; landmarkModel?: string; fit: FitState | null; busy: boolean; busyKind?:import('./workflowUI').ActivityKind; error: string | null; viewerReady: boolean }
type GuideVisit={route:GuideRoute;step:GuideStep|null}
export type GuideStep = { key: string; target: string; title: string; text: string }
export function guideStep(s: GuideState, route: GuideRoute): GuideStep {const step=pairingGuideStep(s,route);if(step.key==='welcome')return step;return {...step,title:imagingText(step.title),text:imagingText(step.text)}}
function pairingGuideStep(s: GuideState, route: GuideRoute): GuideStep {
  const priority=projectGuidePriority({...s,pendingProposal:Boolean(s.fit?.proposal)},route)
  switch(priority.key){
   case 'welcome':return {key:'welcome',target:'.welcome-pairings',title:'Choose your image pairing',text:'**Choose a pairing:** click MRI–CT, T1–T2, T1–T1 or MRI–EPI on the welcome screen to match the images you want to align. This sets the image labels and opens the workspace; it does not load data or run registration.\n\n**Next:** the guide shows you how to select the two images and review their working resolution.\n\n**Saved work:** use Open saved session on this screen to resume an existing session.'}
   case 'viewer':return {key:'viewer',target:'#browser-compatibility',title:'Viewer needs attention',text:'The image viewer needs WebGL2. Resolve the browser warning before loading and navigating images. You can hide this guide and resume it later.'}
   case 'mri':return {key:'mri',target:'#mri-file',title:'Load your MRI',text:'Select MRI and choose your MRI volume; the app inspects its header before loading voxel data. Paired formats may need both files. The guide continues after successful selection; canceling the file picker keeps this step open.'}
   case 'ct':return {key:'ct',target:'#ct-file',title:'Load your CT',text:'Select CT and choose the corresponding CT volume. Next, review working resolution for both images. Already-loaded MRI data stays in place. You can also use the workstation browser to load either image.'}
   case 'prepare':return {key:'prepare',target:s.preparation?.open?'#preparation-title':'#working-resolution',title:'Review resolution',text:'**Compare the inputs.** Open Resolution and review native voxel spacing and estimated memory.\n\n**Choose working resolution.** Use recommended or custom spacing. For multi-volume files, also select a volume/time point.\n\n**Prepare workspace.** The guide continues only after both images are prepared successfully.'}
   case 'working':{const kind=priority.activityKind;if(kind!=='registration')return {key:'working',target:'#alignment-progress',title:kind==='processing'?'Preprocessing in progress':kind==='preparation'?'Preparing working images':kind==='export'?'Export in progress':'Refreshing existing overlay',text:kind==='processing'?'Updating the selected working image only. No registration is being run; the current transform and landmarks remain unchanged.':kind==='display'?'Redrawing with the unchanged transform. No registration is being run.':'Wait for this task to finish; registration is not running.'}
    return {key:'working',target:'#alignment-progress',title:'Alignment in progress',text:'The progress strip shows the current stage. Wait for the preview, then review the result before continuing. You can hide this guide while the program works.'}}
   case 'proposal':{const fit=s.fit!;return {key:'proposal',target:route==='manual'?'#manual-proposal-actions':fit.proposal!.kind==='spline'?'#spline-actions':isLocalRegistration(fit)?'#refine-actions-followup':'#refine-actions',title:fit.proposal!.kind==='spline'?'Review the nonlinear spline proposal':fit.proposal!.kind==='jip'?`Review the ${fit.jipRegistration?.settings.engine==='itk'?'ITK':'JIP'} registration proposal`:fit.automaticRegistration?.operation==='global'?'Review the image-based registration proposal':'Review the refinement proposal',text:'**Review the preview.** Inspect the overlay before deciding. Manual adjustment and new registrations wait for your decision.\n\n**Accept** keeps this result. **Reject** restores the previous alignment, or clears the preview if there was none. The next-step choices return after either decision.\n\n**Other options:** decide and go directly to manual adjustment, or save a session with the pending preview.'}}
  }
  if(route==='registration')return {key:'registration',target:'#guided-setup',title:'Initial processing complete — choose registration',text:'Your prepared working images are ready. Choose your next step:\n\n**Landmarks:** use corresponding anatomical points.\n\n**Independent image-based registration:** align the images automatically without landmarks.\n\n**Manual adjustment:** position the images yourself.\n\nMasking and cropping do not perform registration. You can return to **Initial processing** whenever needed.'}
  if (route === 'choose') return {key:'choose',target:'#guided-setup',title:'How would you like to begin?',text:'**Optional Initial processing** helps prepare the images: adjust display contrast to see anatomy, create/apply masks to remove unwanted intensities, crop irrelevant regions, or edit artifacts voxel by voxel. Display changes affect viewing only; applied masks, crops and voxel edits change the working data used for registration.\n\n**Landmarks** are useful when orientations differ greatly.\n\n**Manual adjustment** lets you start from the images’ existing physical coordinates and adjust the alignment yourself.\n\n**Independent image-based registration** aligns the images automatically without landmarks.'}
  const processingSteps:Record<string,GuideStep>={
    processing:{key:'processing',target:'#processing-display',title:'Initial processing · image display',text:'Adjust MRI and CT contrast and intensity endpoints for comfortable viewing. These controls affect display only. Continue when ready or skip this optional step.'},
    'processing-mask':{key:'processing-mask',target:'#processing-masking',title:'Initial processing · masking',text:'**Preview:** set independent mask thresholds and review the green selection. Optionally fill enclosed holes.\n\n**Create:** select Create/update binary mask and review the result.\n\n**Apply (optional):** apply the generated mask to the working image. Creating or applying a mask does not run registration. Continue when finished.'},
    'processing-crop':{key:'processing-crop',target:'#processing-cropping',title:'Initial processing · cropping',text:'Choose MRI or CT, draw a crop in orthogonal views or edit the physical bounds, then Apply crop if desired. Finish the tool and Continue when ready. Original files remain unchanged.'},
    'processing-voxels':{key:'processing-voxels',target:'#processing-editing',title:'Initial processing · voxel editing',text:'**Edit target:** choose MRI image, CT image or a generated binary mask.\n\n**Brush radius:** enter a whole number. 0 edits one voxel; larger values paint a 3D neighborhood.\n\n**Images:** replace intensity (0 acts as an eraser), or include/exclude voxels from registration. **Masks:** Erase writes 0 and Paint writes 1, without changing image intensities.\n\n**Paint and apply:** click Edit voxels, then click or drag. Apply edits or Finish commits the preview once. Undo/Redo remain available.'}
  }
  if(processingSteps[route])return processingSteps[route]
  if(route==='inspect')return {key:'inspect',target:'#alignment-display-panel',title:'Inspect alignment',text:'Use Alignment display to compare the overlay in all three planes. Adjust moving-image opacity or enable its 25–75% animation. Choose the next step when you have finished inspecting.'}
  if (route === 'landmarks') {
    const complete=s.landmarks.filter(l=>l.enabled&&l.mri&&l.ct).length
    const pending=s.landmarks.find(l=>l.id===s.selectedId&&l.enabled&&(!l.mri||!l.ct)) ?? s.landmarks.find(l=>l.enabled&&(!l.mri||!l.ct))
    if(pending) {
      const modality=!pending.mri?'mri':'ct'
      return {key:`point-${pending.id}-${modality}`,target:`[data-landmark-id="${pending.id}"] .set-${modality}`,title:`Landmark ${pending.id}: set ${modality.toUpperCase()}`,text:`Navigate the ${modality.toUpperCase()} views to ${modality==='mri'?'a recognizable anatomical point':'the same anatomical point you selected in the MRI'}, then press Set in this landmark’s ${modality.toUpperCase()} row. Click to move the crosshair; Shift + scroll changes slices. ${complete} complete pair${complete===1?'':'s'} so far. ${modality==='ct'?'Choose the matching anatomy, not matching voxel numbers.':''}`}
    }
    if(s.fit && hasLandmarkBasis(s.fit) && !s.fit.landmarksChanged && (!s.landmarkModel||s.fit.model===s.landmarkModel)) return {key:'review',target:'#review-label',title:'Review the landmark alignment',text:'Inspect the overlay in all three planes. Review the RMS error, scales, determinant, shear, and any warnings. A low residual alone does not guarantee anatomical alignment. You can retry another model, adjust manually, or continue to refinement.'}

    return {key:'pairs',target:'#new-pair',title:complete?'Add or review landmark pairs':'Create your first landmark pair',text:`**Landmark count:** ${complete} complete pair${complete===1?'':'s'}; the selected model requires at least ${s.minimumPairs}. Reaching the minimum does not mean you are finished. Keep adding or reviewing pairs until you are satisfied, then choose Apply landmark transformation yourself.\n\n**Rigid alignment:** five well-distributed, accurately paired points are a practical starting set: anterior midline, posterior midline, superior midline, and left and right lateral points around the middle of the anterior–posterior extent.\n\n**Review:** use matching anatomy visible in both images and inspect the final overlay; point count alone does not ensure a good fit.`}
  }
  if(route==='spline')return {key:'spline',target:'#spline-smoothing',title:'Optional nonlinear landmark spline',text:'**Landmark-based:** this fits paired landmarks, not image intensities.\n\n**Requirements:** use at least 12 well-distributed pairs (20+ recommended) and review smoothing and distortion warnings.\n\n**Fit and review:** select Fit landmark spline, then accept or reject the proposal before image-based landmark refinement.'}
  if(route==='automatic'&&!s.fit)return {key:'initial',target:'#auto-linear',title:'Run linear registration',text:'**Choose a method.** Start with Linear registration for position, orientation and, when needed, scale or shear. JIP nonlinear registration adds deformation after you accept a linear alignment. ITK nonlinear B-spline is a CPU alternative that adds deformation to the accepted linear alignment. Both nonlinear methods require that first linear fit.\n\n**Prepared images:** if the images already have similar orientation and the fixed image has zero background, use Use prepared-image settings in Linear registration. It selects a staged nearby fit and foreground mean-normalized MI. Review before running.\n\n**Defaults:** choose a software settings profile at the top of Linear registration, inspect its source and differences, then click Apply defaults in the main program. You can keep your chosen model and edit every parameter.\n\n**Linear settings:** check transform direction and choose rigid, similarity, anisotropic scale, affine, or custom groups.\n\n**Run and review:** click Run linear registration in the main program, watch progress and inspect the proposal before accepting. Wider orientation search can take longer.'}
  if(route==='manual'&&!s.fit) return {key:'initial',target:'#manual-start-geometry',title:'Begin manual alignment',text:'Select Begin manual alignment to create a starting overlay, then move, rotate, scale, or shear MRI or CT using the manual controls. The images may initially overlap poorly. If you have already aligned the images, the manual controls adjust that alignment directly.'}
  const steps: Record<string, GuideStep> = {
    automatic:{key:'initial-review',target:'#auto-linear',title:'Image-based registration options',text:'**Linear registration** proposes position, orientation and optional scale/shear adjustments. **JIP nonlinear registration** adds deformation after you accept a linear alignment. **ITK nonlinear B-spline** is a CPU alternative for adding deformation to the accepted linear alignment. Both nonlinear methods preserve that linear base.\n\nUse the matching section in the main program, then review the proposed overlay, scale, shear and determinant before accepting. You can also adjust manually, refine a landmark alignment, or save your work.'},
    manual:{key:'manual',target:'.manual-targets',title:'Adjust the overlay manually',text:'**Choose an image:** select Adjust MRI or Adjust CT.\n\n**Adjust:** use translation, rotation, axis scale or shear. The other image defines the preview grid; export direction stays unchanged. Optionally enable the transformed source-row preview.\n\n**Review:** inspect all three planes. Choose Continue when ready; no movement is required.'},
    review:{key:'review',target:'#guided-setup',title:'What would you like to do next?',text:'**Inspect** the overlay and diagnostics before choosing the next step.\n\n**Continue aligning:** adjust manually, refine again, retry landmarks, or run a new automatic registration search. Automatic registration searches for a new starting alignment; refinement improves the existing alignment.\n\n**Save or export** when you are ready.'},
    windows:{key:'windows',target:'#define-windows',title:'Optional: restrict refinement',text:'**Purpose:** these shared optimization regions restrict the anatomy used for landmark refinement and independent image-based registration.\n\n**Define regions:** select Define windows and drag rectangles in the relevant image planes. Select Define windows again to finish.\n\n**Optional:** skip this to use the full geometric overlap when no regions are defined.'},
    refinement:{key:s.fit?.proposal?'proposal':'refine',target:s.fit?.proposal?'#refine-actions-followup':'#local-auto-linear',title:s.fit?.proposal?'Review the refinement proposal':'Try automatic refinement',text:s.fit?.proposal?'The proposed overlay is a preview. Inspect it, then select Accept registration or Reject. Export requires this decision. The guide waits until you decide.':s.busy?'Refinement is running. Wait for the result; the guide will show the review step when a proposal is ready.':'**Settings:** in Landmark refinement, choose Tight, Standard, or Custom limits and start steps, then run refinement.\n\n**Purpose:** this is a constrained rigid image-based adjustment of your landmark-derived alignment; it preserves its scale, shear or spline.\n\n**If it fails:** inspect overlap/windows and adjust or retry. You can skip refinement.'},
    export:{key:s.fit?.proposal?'proposal':'export',target:s.fit?'#export-open':'#save-session',title:s.fit?.proposal?'Review the pending proposal':'Save your work',text:s.fit?.proposal?'Accept or reject the pending refinement before saving registration files. Your session can preserve the proposal for later review.':s.fit?'**Export options:** select Export to save the aligned image, registration matrices, or a reloadable session.\n\n**Direction:** exports follow the chosen moving-to-fixed direction.\n\n**Check the result:** keep the original images and visually check the exported result.':'Select Save session to preserve your images’ geometry and landmarks. You can resume the session later after loading the same images. Registration/image export becomes available once an alignment is initialized.'},
    done:{key:'done',target:'#guided-setup',title:'You are ready to continue',text:'You can return to any tool or restart Guided setup at any time. Ending this guide keeps all images, landmarks, and alignment changes.'},
  }
  return steps[route] ?? steps.review
}
export function installGuidedSetup(readState: () => GuideState) {
  const beganWithoutImages=!readState().mri||!readState().ct
  const modalOpen=()=>Boolean(document.querySelector('.modal:not(.hidden):not([hidden]),dialog[open]'))
  const start=document.querySelector<HTMLButtonElement>('#guided-setup')!
  const card=document.createElement('section')
  card.id='guided-card';card.className='guided-card hidden';card.setAttribute('role','dialog');card.setAttribute('aria-modal','false');card.setAttribute('aria-labelledby','guided-title');card.setAttribute('aria-describedby','guided-text')
  card.innerHTML='<div class="guided-head" id="guided-move" tabindex="0" aria-label="Move guide: drag this header or use arrow keys"><span>Guided setup</span><button type="button" aria-label="Hide guide; resume later" id="guided-close">×</button></div><p class="guided-help">Drag this header anywhere, including out of the dock. Follow highlighted control or a new step restores automatic placement. × hides; Resume guide continues.</p><h2 id="guided-title" tabindex="-1"></h2><div id="guided-text" aria-live="polite"></div><p id="guided-error" class="guided-error"></p><div id="guided-actions"></div><div class="guided-tools"><button type="button" id="guided-back">Back</button><button type="button" id="guided-anchor">Follow highlighted control</button><button type="button" id="guided-restart" title="Restart instructions; keep loaded images and registration work">Restart guide</button></div>'
  document.body.appendChild(card)
  let route:GuideRoute='choose', active=false, timer:number|undefined, last='', highlighted:HTMLElement|null=null, descriptor:string|null=null
  const resume=document.createElement('button');resume.id='guided-resume';resume.type='button';resume.className='guided-resume hidden';resume.textContent='Resume guide';(document.querySelector('.statusbar')??document.body).appendChild(resume)
  const startLabel=start.textContent
  let paused=false, manualPosition:{x:number;y:number}|null=null, drag:{id:number;x:number;y:number;left:number;top:number}|null=null
  let history:GuideVisit[]=[],presented:GuideVisit|null=null,backStep:GuideStep|null=null;
  const remember=(visit:GuideVisit)=>{const previous=history.at(-1);if(previous?.route!==visit.route||previous?.step?.key!==visit.step?.key)history.push(visit);history=history.slice(-100)};let hadProposal=false;let syncing=false;let editingWindows=false;let landmarkBaseline:string|undefined
  let requested:GuideStep|null=null
  let pendingProcessingRoute:GuideRoute|null=null
  const processingActive=()=>['#processing-crop-draw','#processing-voxel'].some(selector=>document.querySelector(selector)?.getAttribute('aria-pressed')==='true')
  const continueProcessing=(next:GuideRoute)=>{if(processingActive()){pendingProcessingRoute=next;pointTo('#processing-stop','Finish the processing tool first','Click Finish processing tool in the main program. If voxel edits are pending, this applies them. The guide continues after the tool finishes.')}else go(next)}
  const pointTo=(target:string,title:string,text='Click the highlighted control in the main program. The guide waits for your action.')=>{requested={key:target==='#new-pair'?'teach-add-pair':'teach-control-'+target,target,title,text};last='';refresh()}
  let pulseTarget:HTMLElement|null=null,pulseKey='',pulseTimer:number|undefined
  const replayPulse=()=>{if(!highlighted)return;window.clearTimeout(pulseTimer);highlighted.classList.remove('guided-pulse');if(window.matchMedia?.('(prefers-reduced-motion: reduce)').matches)return;void highlighted.offsetWidth;highlighted.classList.add('guided-pulse');const target=highlighted;pulseTimer=window.setTimeout(()=>target.classList.remove('guided-pulse'),1800)}
  const unhighlight=()=>{if(highlighted){highlighted.classList.remove('guided-target','guided-pulse');if(descriptor===null)highlighted.removeAttribute('aria-describedby');else highlighted.setAttribute('aria-describedby',descriptor)}highlighted=null}
  let savedGuide='',resetting=false
  const persist=()=>{if(resetting||!getPreferences().guide.remember)return;const state={route,history:history.map(v=>v.route),manualPosition,paused:active||paused};const text=JSON.stringify(state);if(text===savedGuide)return;savedGuide=text;void preferenceStorage.write('guide',state).catch(()=>{card.querySelector('#guided-error')!.textContent='Guide progress could not be saved.'})}
  const resize=()=>window.dispatchEvent(new window.Event('resize'))
  const close=()=>{paused=active||paused;active=false;drag=null;window.clearInterval(timer);window.clearTimeout(pulseTimer);pulseTarget=null;pulseKey='';document.body.classList.remove('guide-auto-docked','guide-manual-floating');card.classList.add('hidden');unhighlight();start.setAttribute('aria-expanded','false');start.textContent=paused?'Resume guide':startLabel;resume.classList.toggle('hidden',!paused);if(!modalOpen())start.focus();persist();resize()}
  const finish=()=>{pendingProcessingRoute=null;requested=null;close();paused=false;route='choose';history=[];presented=null;backStep=null;landmarkBaseline=undefined;manualPosition=null;hadProposal=false;start.textContent=startLabel;resume.classList.add('hidden');persist()}

  const selectOperation=(next:GuideRoute)=>{const operation=document.querySelector<HTMLSelectElement>('#intensity-operation');if(operation&&['automatic','refinement','spline'].includes(next)){const previous=syncing;syncing=true;const value=next==='automatic'?'global':'local';if(operation.value!==value){operation.value=value;operation.dispatchEvent(new window.Event('change'))}syncing=previous}}
  // Route choices change instructions only. Native panel clicks select their own operation.
  const focusRoute=(next:GuideRoute,refinementWindows=false)=>{selectOperation(next==='windows'&&refinementWindows?'refinement':next)}
  const go=(next:GuideRoute,sync=true)=>{if(processingActive()&&['landmarks','manual','automatic','refinement','windows','spline'].includes(next)){pendingProcessingRoute=next;pointTo('#processing-stop','Finish the processing tool first','Click Finish processing tool in the main program before starting a registration workflow. Pending voxel edits are applied only when you explicitly finish the tool.');return}pendingProcessingRoute=null;requested=null;if(sync)focusRoute(next,true);if(next==='landmarks')landmarkBaseline=readState().fit?.fittedAt;if(next!=='windows')editingWindows=false;backStep=null;route=next;last='';refresh();persist()}
  const position=()=>{
    if(!active||modalOpen())return
    if(manualPosition){
      document.body.classList.add('guide-manual-floating')
      const width=Math.min(350,window.innerWidth-24);card.style.width=`${width}px`
      const height=Math.min(card.getBoundingClientRect().height||320,window.innerHeight-24)
      manualPosition=clampGuidePosition({width:window.innerWidth,height:window.innerHeight},{width,height},manualPosition)
      card.dataset.side='floating';card.style.left=`${manualPosition.x}px`;card.style.top=`${manualPosition.y}px`;return
    }
    document.body.classList.remove('guide-manual-floating')
    const dock=()=>{document.documentElement.style.setProperty('--guide-top',`${(document.querySelector('main')?.getBoundingClientRect().top||140)+8}px`);card.dataset.side='docked'}
    if(getPreferences().guide.dock||document.body.classList.contains('guide-auto-docked')){dock();return}
    const target=highlighted?.getBoundingClientRect()??start.getBoundingClientRect(),width=Math.min(350,window.innerWidth-24)
    card.style.width=`${width}px`
    const height=Math.min(card.getBoundingClientRect().height||320,window.innerHeight-24)
    const stepKey=card.dataset.step??''
    let selector=''
    if(stepKey.startsWith('point-'))selector=`.view-card[data-modality="${stepKey.endsWith('-mri')?'mri':'ct'}"]`
    else if(stepKey.startsWith('processing'))selector='.view-card[data-modality]'
    else if(stepKey==='windows')selector=document.querySelector<HTMLSelectElement>('#window-target')?.value==='mri'?'.view-card[data-modality="mri"]':document.querySelector<HTMLSelectElement>('#window-target')?.value==='ct'?'.view-card[data-modality="ct"]':'.view-card[data-modality]'
    else if(['review','manual','proposal','inspect'].includes(stepKey)||Boolean(readState().fit?.proposal))selector='.review-card'
    const landmarkRow=highlighted?.closest('.landmark-row')?.getBoundingClientRect();
    const protectedRects=[target,...(landmarkRow?[landmarkRow]:[]),...(selector?[...document.querySelectorAll<HTMLElement>(selector)].map(el=>el.getBoundingClientRect()).filter(r=>r.width>0&&r.height>0):[])]
    const placement=placeGuide({width:window.innerWidth,height:window.innerHeight},{width,height},target,protectedRects)
    if(!placement){document.body.classList.add('guide-auto-docked');dock();resize();highlighted?.scrollIntoView?.({block:'nearest',inline:'nearest'});return}
    card.dataset.side='automatic';card.style.left=`${placement.x}px`;card.style.top=`${placement.y}px`
  }
  function refresh(){
    if(!active||modalOpen())return
    const s=readState()
    // A newly delivered result invalidates older location/history instructions.
    // Keep subsequent Show Accept/Reject requests until the user decides.
    if(s.fit?.proposal&&!hadProposal){pendingProcessingRoute=null;requested=null;backStep=null;if(route==='choose'||route==='done')route='review'}
    if(pendingProcessingRoute&&!processingActive()&&!s.busy){const next=pendingProcessingRoute;pendingProcessingRoute=null;go(next);return}
    // Resolve a decision before the empty-alignment fallback clears its latch.
    if(hadProposal && !s.fit?.proposal && !s.busy){hadProposal=false;go('review',false);return}
    if(!s.fit&&!s.busy&&(['review','refinement','spline'].includes(route)||(route==='windows'&&document.querySelector<HTMLSelectElement>('#intensity-operation')?.value==='local'))){route='choose';history=[];presented=null;backStep=null;hadProposal=false;last='';persist()}
    if(!s.busy&&!s.fit?.proposal&&route==='landmarks'&&s.fit&&s.fit.fittedAt!==landmarkBaseline&&!s.fit.landmarksChanged){go('review',false);return}
    if(s.fit?.proposal)hadProposal=true
    const defining=document.querySelector('#define-windows')?.getAttribute('aria-pressed')==='true'
    if(route==='windows'&&defining)editingWindows=true
    if(route==='windows'&&editingWindows&&!defining&&!s.busy){editingWindows=false;go(document.querySelector<HTMLSelectElement>('#intensity-operation')?.value==='global'?'automatic':'refinement',false);return}
    const current=guideStep(route==='landmarks'&&!s.fit?.proposal&&s.fit?.fittedAt===landmarkBaseline?{...s,fit:null}:s,route);
    // Entry prerequisites cannot be bypassed by remembered routes or Back.
    const baseStep=isGuidePrerequisite(current.key)?current:(requested??backStep??current);
    if(baseStep.target==='.manual-targets'&&!document.querySelector('.manual-targets'))baseStep.target='#manual-panel'
    if(baseStep.key==='windows'){if(document.querySelector<HTMLSelectElement>('#intensity-operation')?.value==='global')baseStep.title='Optional: restrict image-based registration';else if(document.querySelector('#define-windows-followup'))baseStep.target='#define-windows-followup'}
    const step=guideNavigationStep(baseStep), key=JSON.stringify([step.key,step.target,step.title,step.text,defining,route,s.minimumPairs,s.busy,hasLandmarkBasis(s.fit),s.fit?.fittedAt,Boolean(s.fit?.proposal),s.fit?.proposal?.kind])
    if(step.key==='windows'){if(document.querySelector<HTMLSelectElement>('#intensity-operation')?.value==='global')step.title='Optional: restrict image-based registration';else if(document.querySelector('#define-windows-followup'))step.target='#define-windows-followup'}
    card.querySelector('#guided-error')!.textContent=s.error?`The tool reported: ${s.error}`:''
    // Landmark rows are rebuilt on animation frames. Resolve the current target
    // before the fast path: a temporary missing button must never pin the guide
    // to its launch button, or retain a detached row after a render.
    let anchor=document.querySelector<HTMLElement>(step.target)
    if(anchor?.matches('input[type=file]'))anchor=anchor.closest('label')
    if(key===last&&highlighted===anchor&&anchor?.isConnected){position();return}
    if(presented&&!isGuidePrerequisite(presented.step?.key??'')&&!presented.step?.key.startsWith('teach-')&&(presented.route!==route||presented.step?.key!==step.key)&&!backStep)remember(presented)
    presented={route,step:{...step}};persist()
    if(card.dataset.step!==step.key){manualPosition=null;const docked=document.body.classList.contains('guide-auto-docked')||document.body.classList.contains('guide-manual-floating');document.body.classList.remove('guide-auto-docked','guide-manual-floating');if(docked)window.setTimeout(resize,0)}
    card.dataset.step=step.key;last=key
    unhighlight()
    highlighted=anchor
    if(highlighted){descriptor=highlighted.getAttribute('aria-describedby');highlighted.classList.add('guided-target');highlighted.setAttribute('aria-describedby','guided-text')
    if(!pulseTarget||pulseKey!==step.key){window.clearTimeout(pulseTimer);pulseTarget=highlighted;pulseKey=step.key;if(!window.matchMedia?.('(prefers-reduced-motion: reduce)').matches){highlighted.classList.add('guided-pulse');const target=highlighted;pulseTimer=window.setTimeout(()=>target.classList.remove('guided-pulse'),1800)}}
    highlighted.scrollIntoView?.({block:'nearest',inline:'nearest'})}
    card.querySelector('#guided-title')!.textContent=step.key==='windows'&&defining?'Draw optimization windows':step.title;renderGuideText(card.querySelector<HTMLElement>('#guided-text')!,step.key==='windows'&&defining?'**Draw regions** in the image planes you want to restrict. Keep selecting windows until you are ready.\n\n**Finish** by clicking Define windows again in the main program; the guide will continue automatically.':step.text)
    const actions=card.querySelector('#guided-actions')!;actions.replaceChildren()
    const button=(label:string,action:()=>void,primary=false)=>{const b=document.createElement('button');b.type='button';b.textContent=label;if(primary)b.className='primary';b.addEventListener('click',()=>{syncing=true;try{action()}finally{syncing=false}});actions.appendChild(b)}
    const methodLocations=()=>{if(route!=='automatic')return;for(const [label,target] of [['Linear','#linear-defaults-select'],['JIP nonlinear','#jip-metric'],['ITK nonlinear','#itk-metric']])if(document.querySelector(target))button(`Show ${label} registration`,()=>pointTo(target,`Review ${label} registration settings in the main program`))}
        if(step.key==='choose'||step.key==='registration'){
      if(step.key==='registration'&&s.fit)button('Continue with current alignment',()=>go(s.fit?.proposal?'refinement':'review'),true)
      if(step.key==='choose'&&document.querySelector('#initial-processing-panel'))button('Review initial processing',()=>go('processing'));button(step.key==='registration'?'Start with landmarks':'Guide me through landmarks',()=>go('landmarks'));button(step.key==='registration'?'Use image-based registration':'Skip landmarks · automatic registration',()=>go('automatic'));button(step.key==='registration'?'Use manual adjustment':'Start with manual adjustment',()=>go('manual'))
      if(step.key==='choose'&&s.fit)button(s.fit.proposal?'Review pending proposal':'Review existing alignment',()=>go(s.fit?.proposal?'refinement':'review'))
    }else if(step.key.startsWith('teach-open-')||step.key.startsWith('teach-close-')){button('Show heading',()=>{manualPosition=null;position();highlighted?.focus({preventScroll:true});replayPulse()},true)}else if(step.key.startsWith('teach-control-')||step.key==='teach-add-pair'){button('Show control',()=>{manualPosition=null;highlighted?.focus({preventScroll:true});position()},true);button('Return to instructions',()=>{pendingProcessingRoute=null;requested=null;last='';refresh()})}else if(step.key.startsWith('processing')){const next:Record<string,GuideRoute>={processing:'processing-mask','processing-mask':'processing-crop','processing-crop':'processing-voxels','processing-voxels':'registration'};button(step.key==='processing-voxels'?'Continue to registration choices':'Continue',()=>{continueProcessing(next[step.key])},true);button('Skip remaining processing',()=>{continueProcessing('registration')});actions.querySelector<HTMLButtonElement>('button')!.disabled=s.busy}else if(step.key.startsWith('point-')){
      const match=/^point-(\d+)-(mri|ct)$/.exec(step.key),point=match?s.landmarks.find(l=>l.id===Number(match[1])):null,modality=match?.[2] as 'mri'|'ct'|undefined
      const set=()=>{manualPosition=null;last='';refresh();highlighted?.focus({preventScroll:true})}
      if(point&&modality)button(`Show ${point[modality]?'Replace':'Set'} ${modality.toUpperCase()} control`,set,true)
      if(backStep)button('Continue with current progress',()=>{backStep=null;last='';refresh()})
      button('Use another registration method',()=>go('choose'))
    }else if(backStep&&step.key==='pairs'){
      button('Continue with current progress',()=>{backStep=null;last='';refresh()})
      button('Use another registration method',()=>go('choose'))
    }else if(['pairs','fit'].includes(step.key)){
      button('Show Add landmark pair',()=>pointTo('#new-pair','Add a pair in the main toolbar'))
      button('Show landmark alignment',()=>pointTo('#fit-side','Run landmark alignment in the main program'))
      const pairButtons=actions.querySelectorAll<HTMLButtonElement>('button')
      pairButtons[1].disabled=s.busy||s.landmarks.filter(l=>l.enabled&&l.mri&&l.ct).length<s.minimumPairs
      button('Use another registration method',()=>go('choose'))
    }
    else if(step.key==='initial'){methodLocations();button('Use landmarks instead',()=>go('landmarks'));if(route==='automatic'){button('Guide optimization regions',()=>go('windows',false));}if(route==='automatic')button('Skip automatic · manual setup',()=>go('manual'));button('Skip registration · save session',()=>go('export'))}
    else if(['review','manual','initial-review','inspect'].includes(step.key)){
      if(step.key==='initial-review')methodLocations()
      if(step.key==='review')button('Inspect alignment',()=>go('inspect'),true)
      if(step.key==='inspect'||step.key==='manual')button('Choose next step',()=>go('review'),true)
      if(hasLandmarkBasis(s.fit))button(step.key==='manual'?'Continue to refinement options':'Guide automatic refinement',()=>go('windows'))
      if(hasLandmarkBasis(s.fit)&&document.querySelector('#run-landmark-spline'))button('Explore nonlinear landmark spline',()=>go('spline'))
      if(route==='automatic')button('Guide optimization regions',()=>go('windows',false))
      if(step.key!=='manual')button('Guide manual adjustment',()=>go('manual'));if(route!=='automatic')button('Guide automatic registration',()=>go('automatic'))
      button('Save or export',()=>go('export'));button('Retry landmarks',()=>go('landmarks'))
    }else if(step.key==='windows'){const global=document.querySelector<HTMLSelectElement>('#intensity-operation')?.value==='global';button(global?'Continue to image-based registration':'Continue to automatic refinement',()=>{const defining=document.querySelector('#define-windows')?.getAttribute('aria-pressed')==='true';if(defining)pointTo(global?'#define-windows':document.querySelector('#define-windows-followup')?'#define-windows-followup':'#define-windows','Finish defining windows in the main program');else go(global?'automatic':'refinement')});button('Skip refinement',()=>go('export'))}
    else if(step.key==='proposal'){
      button('Show alignment display',()=>pointTo('#alignment-display-panel','Inspect the proposed alignment'),true)
      button('Show Accept control',()=>{const control=proposalDecisionControl(s.fit,true);if(control)pointTo('#'+control.id,'Accept the proposal in the main program')})
      button('Show Reject control',()=>{const control=proposalDecisionControl(s.fit,false);if(control)pointTo('#'+control.id,'Reject the proposal in the main program')})
      button('Show Save session',()=>pointTo('#save-session','Save the pending proposal in the main toolbar'))
    }
    else if(step.key==='spline'){button('Continue to landmark refinement',()=>go('windows'));button('Save or export',()=>go('export'))}
    else if(step.key==='refine'&&!s.busy){button('Skip refinement',()=>go('export'));button('Adjust manually first',()=>go('manual'))}
    else if(step.key==='export'||step.key==='done')button('Finish guide',finish,true)
    else if(step.key==='mri'||step.key==='ct')button('Hide guide for now',close)
    const back=card.querySelector<HTMLButtonElement>('#guided-back')!;back.disabled=history.length===0 || s.busy || Boolean(s.fit?.proposal)
    position()
  }
  const observe=(event:Event)=>{
    if(!active||syncing||readState().busy)return
    const target=event.target instanceof window.HTMLElement?event.target:null
    if(!target||card.contains(target)||target===start||target===resume)return
    if(requested&&event.type==='click'&&target.closest(requested.target)){requested=null;last=''}
    const button=target.closest<HTMLElement>('button'),id=button?.id??target.id
    const pointAction=Boolean(button?.matches('.set-mri,.set-ct,.select,.delete,.enabled'));
    const workflow=button?.dataset.workflow,priorFit=readState().fit?.fittedAt
    const operation=target.id==='intensity-operation'&&event.type==='change'&&(event as CustomEvent).detail?.source!=='disclosure'
    const action=event.type==='click'&&(workflow||['new-pair','fit-side','run-landmark-spline','local-auto-linear','auto-linear','jip-run','itk-run','define-windows','define-windows-followup','manual-start-geometry','use-image-geometry'].includes(id))
    queueMicrotask(()=>{
      if(!active||syncing)return
      if(pointAction||action||operation){backStep=null;requested=null}
      if(requested&&event.type==='click'&&target.closest(requested.target)){requested=null;last=''}
      if(operation){go((target as HTMLSelectElement).value==='global'?'automatic':'refinement',false)}
      else if(action){
        let next:GuideRoute|undefined
        if(workflow==='landmarks'||id==='new-pair'||id==='fit-side')next='landmarks'
        else if(workflow==='manual'||id==='manual-start-geometry'||id==='use-image-geometry')next='manual'
        else if(id==='define-windows'||id==='define-windows-followup')next=button?.getAttribute('aria-pressed')!=='false'?'windows':document.querySelector<HTMLSelectElement>('#intensity-operation')?.value==='global'?'automatic':'refinement'
        else if(id==='run-landmark-spline')next='spline'
        else if(id==='local-auto-linear')next='refinement'
        else if(workflow==='followup')next='refinement'
        else if(workflow==='automatic'||id==='auto-linear'||id==='jip-run'||id==='itk-run')next=document.querySelector<HTMLSelectElement>('#intensity-operation')?.value==='local'?'refinement':'automatic'
        if(next&&next!==route){go(next,false);if(next==='landmarks'){landmarkBaseline=priorFit;last=''}}
      }
      refresh()
    })
  }
  document.addEventListener('toggle',()=>{if(active){last='';refresh()}},true)
  document.addEventListener('click',observe,true)
  document.addEventListener('change',observe,true)
  document.addEventListener('input',observe,true)
  start.setAttribute('aria-haspopup','dialog');start.setAttribute('aria-expanded','false')
  const show=()=>{if(active){close();return}active=true;if(!paused){pendingProcessingRoute=null;requested=null;route='choose';history=[];presented=null;backStep=null;hadProposal=false;editingWindows=false;landmarkBaseline=undefined}paused=false;focusRoute(route);last='';card.classList.remove('hidden');resume.classList.add('hidden');start.textContent=startLabel;start.setAttribute('aria-expanded','true');refresh();if(!modalOpen())card.querySelector<HTMLElement>('#guided-title')!.focus();timer=window.setInterval(refresh,250);resize()}
  start.addEventListener('click',show);resume.addEventListener('click',show)
  card.querySelector('#guided-restart')!.addEventListener('click',()=>{pendingProcessingRoute=null;requested=null;route='choose';history=[];presented=null;backStep=null;hadProposal=false;editingWindows=false;landmarkBaseline=undefined;last='';manualPosition=null;refresh();persist()})
  window.addEventListener('align-workspace-reset',()=>{resetting=true;pendingProcessingRoute=null;requested=null;close();paused=false;route='choose';history=[];presented=null;backStep=null;hadProposal=false;editingWindows=false;landmarkBaseline=undefined;start.textContent=startLabel;resume.classList.add('hidden')})
  window.addEventListener('align-workspace-reset-failed',()=>{resetting=false})
  card.querySelector('#guided-anchor')!.addEventListener('click',()=>{manualPosition=null;document.body.classList.remove('guide-manual-floating');last='';refresh();replayPulse();highlighted?.scrollIntoView?.({block:'nearest',inline:'nearest'});highlighted?.focus({preventScroll:true});position();persist()})
  const handle=card.querySelector<HTMLElement>('#guided-move')!
  // A deliberate drag/keyboard move takes precedence over automatic avoidance
  // and docking. Keep only viewport bounds; never funnel it back through placeGuide.
  const floatAtCurrentPosition=()=>{const r=card.getBoundingClientRect(),floating=card.dataset.side==='floating'||card.dataset.side==='automatic',left=floating?Number.parseFloat(card.style.left):r.left,top=floating?Number.parseFloat(card.style.top):r.top,wasDocked=!document.body.classList.contains('guide-manual-floating')&&(getPreferences().guide.dock||document.body.classList.contains('guide-auto-docked'));manualPosition={x:Number.isFinite(left)?left:r.left,y:Number.isFinite(top)?top:r.top};document.body.classList.remove('guide-auto-docked');document.body.classList.add('guide-manual-floating');position();if(wasDocked)resize();return manualPosition!}
  handle.addEventListener('pointerdown',e=>{if(e.button!==0||e.isPrimary===false||drag||(e.target as HTMLElement).closest('button'))return;e.preventDefault();handle.focus();const p=floatAtCurrentPosition();drag={id:e.pointerId,x:e.clientX,y:e.clientY,left:p.x,top:p.y};handle.setPointerCapture?.(e.pointerId)})
  handle.addEventListener('pointermove',e=>{if(!drag||drag.id!==e.pointerId)return;manualPosition={x:drag.left+e.clientX-drag.x,y:drag.top+e.clientY-drag.y};position()})
  const endDrag=(e:PointerEvent)=>{if(!drag||drag.id!==e.pointerId)return;drag=null;persist()}
  handle.addEventListener('pointerup',endDrag);handle.addEventListener('pointercancel',endDrag);handle.addEventListener('lostpointercapture',endDrag)
  handle.addEventListener('keydown',e=>{if(e.target!==handle||!['ArrowLeft','ArrowRight','ArrowUp','ArrowDown'].includes(e.key))return;e.preventDefault();e.stopPropagation();const p=floatAtCurrentPosition(),step=e.shiftKey?40:10;manualPosition={x:p.x+(e.key==='ArrowRight'?step:e.key==='ArrowLeft'?-step:0),y:p.y+(e.key==='ArrowDown'?step:e.key==='ArrowUp'?-step:0)};position();persist()})
  card.querySelector('#guided-close')!.addEventListener('click',close)
  card.querySelector('#guided-back')!.addEventListener('click',()=>{
    pendingProcessingRoute=null;requested=null;const previous=history.pop();if(!previous)return;route=previous.route;backStep=previous.step&&!previous.step.key.startsWith('teach-')?{...previous.step}:null;presented=null
    if(backStep?.key==='pairs')backStep={...backStep,title:'Landmark pair already added',text:'You have already completed this Add pair step. Back preserves all existing pairs; select Continue with current progress to finish the current pair, or Back again to review the preceding point.'}
    const match=backStep&&/^point-(\d+)-(mri|ct)$/.exec(backStep.key)
    if(match){const point=readState().landmarks.find(l=>l.id===Number(match[1])),modality=match[2] as 'mri'|'ct';if(point?.[modality])backStep={...backStep!,title:`Landmark ${point.id}: review ${modality.toUpperCase()}`,text:`This ${modality.toUpperCase()} point is already set. Review it or navigate to a revised position and use Replace. Back changes instructions only; your landmark coordinates are preserved. Continue with current progress when ready.`}}
    focusRoute(route);last='';refresh();persist()
  })
  // Remember modal ownership before its Escape handler hides the dialog.
  const modalKeys=new WeakSet<Event>()
  document.addEventListener('keydown',e=>{if(modalOpen())modalKeys.add(e)},true)
  document.addEventListener('keydown',e=>{if(active&&e.key==='Escape'&&!e.defaultPrevented&&!modalKeys.has(e)&&!modalOpen()){e.preventDefault();close()}})
  window.addEventListener('align-guide-manual',()=>{if(active&&!readState().fit?.proposal){hadProposal=false;go('manual')}})
  window.addEventListener('align-guide-reset',()=>{pendingProcessingRoute=null;requested=null;hadProposal=false;editingWindows=false;landmarkBaseline=undefined;route='choose';history=[];presented=null;backStep=null;manualPosition=null;last='';savedGuide='';refresh()})
  window.addEventListener('align-preferences-changed',position)
  window.addEventListener('align-preferences-ready',()=>{void (async()=>{if(getPreferences().guide.remember){const saved=await preferenceStorage.read('guide');const allowed=['choose','registration','processing','processing-mask','processing-crop','processing-voxels','inspect','landmarks','automatic','manual','review','windows','refinement','spline','export','done'];if(!resetting&&!active&&saved&&allowed.includes(saved.route)){const empty=!readState().fit&&readState().landmarks.length===0&&(beganWithoutImages||!readState().mri||!readState().ct);route=empty?'choose':saved.route;history=empty?[]:Array.isArray(saved.history)?saved.history.filter((r:string)=>allowed.includes(r)).slice(-20).map((route:GuideRoute)=>({route,step:null})):[];if(saved.manualPosition&&[saved.manualPosition.x,saved.manualPosition.y].every(Number.isFinite))manualPosition=saved.manualPosition;paused=!empty&&Boolean(saved.paused);if(empty)persist();if(paused){start.textContent='Resume guide';resume.classList.remove('hidden')}}}if(!resetting&&getPreferences().guide.firstLaunch&&!active)show()})().catch(()=>{})})
  const landmarkList=document.querySelector('#landmark-list'),landmarkObserver=new window.MutationObserver(()=>{if(card.isConnected)refresh()});if(landmarkList)landmarkObserver.observe(landmarkList,{childList:true});
  // CSS yields immediately, including between two dialogs. Resume instruction
  // projection only after the last dialog closes; never focus/scroll behind it.
  let dialogWasOpen=modalOpen()
  const modalObserver=new window.MutationObserver(()=>{if(!card.isConnected)return;const open=modalOpen();if(open!==dialogWasOpen){dialogWasOpen=open;if(!open){last='';refresh()}}});modalObserver.observe(document.body,{subtree:true,childList:true,attributes:true,attributeFilter:['class','hidden','open']})
  window.addEventListener('pagehide',()=>{landmarkObserver.disconnect();modalObserver.disconnect();window.clearInterval(timer);window.clearTimeout(pulseTimer)},{once:true})
  window.addEventListener('align-welcome-changed',()=>{requested=null;backStep=null;last='';refresh()});
  window.addEventListener('resize',position);document.addEventListener('scroll',position,true)
  return {refresh,close}
}
