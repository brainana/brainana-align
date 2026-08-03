export type WorkflowStage = 'landmarks' | 'linear' | 'landmark-warp' | 'intensity' | 'deformable' | 'review'

export function installWorkflowNavigation(root: ParentNode = document) {
  const tabs = Array.from(root.querySelectorAll<HTMLButtonElement>('[data-workflow-tab]'))
  const panels = Array.from(root.querySelectorAll<HTMLElement>('[data-workflow-panel]'))

  const select = (stage: WorkflowStage) => {
    for (const tab of tabs) {
      const active = tab.dataset.workflowTab === stage
      tab.classList.toggle('active', active)
      tab.setAttribute('aria-selected', String(active))
      tab.tabIndex = active ? 0 : -1
    }
    for (const panel of panels) {
      const active = panel.dataset.workflowPanel === stage
      panel.classList.toggle('active', active)
      panel.setAttribute('aria-hidden', String(!active))
    }
  }

  tabs.forEach((tab, index) => {
    tab.addEventListener('click', () => select(tab.dataset.workflowTab as WorkflowStage))
    tab.addEventListener('keydown', event => {
      if (!['ArrowLeft','ArrowRight','Home','End'].includes(event.key)) return
      event.preventDefault()
      const delta = event.key === 'ArrowRight' ? 1 : -1
      const next = event.key === 'Home' ? tabs[0] : event.key === 'End' ? tabs.at(-1)! : tabs[(index + delta + tabs.length) % tabs.length]
      next.focus()
      select(next.dataset.workflowTab as WorkflowStage)
    })
  })

  select((tabs.find(tab => tab.classList.contains('active'))?.dataset.workflowTab ?? 'landmarks') as WorkflowStage)
  return { select }
}
