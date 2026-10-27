/** Manual placement is bounded only by the viewport, never protected image areas. */
export function clampGuidePosition(viewport:{width:number;height:number},size:{width:number;height:number},point:{x:number;y:number}){const margin=12;return {x:Math.max(margin,Math.min(point.x,viewport.width-size.width-margin)),y:Math.max(margin,Math.min(point.y,viewport.height-size.height-margin))}}
export type GuideRect={left:number;top:number;width:number;height:number}
export function placeGuide(viewport:{width:number;height:number},size:{width:number;height:number},target:GuideRect,protectedRects:GuideRect[],preferred?:{x:number;y:number}|null){
 const margin=12,gap=16,w=size.width,h=size.height
 const clamp=(x:number,y:number)=>clampGuidePosition(viewport,size,{x,y})
 const candidates=[...(preferred?[clamp(preferred.x,preferred.y)]:[]),clamp(target.left,target.top+target.height+gap),clamp(target.left+w>viewport.width?target.left-w-gap:target.left+target.width+gap,target.top),clamp(target.left-w-gap,target.top),clamp(target.left,target.top-h-gap),clamp(viewport.width-w-margin,margin),clamp(viewport.width-w-margin,viewport.height-h-margin),clamp(margin,viewport.height-h-margin),clamp(margin,margin)]
 const overlap=(p:{x:number;y:number},r:GuideRect)=>Math.max(0,Math.min(p.x+w,r.left+r.width+8)-Math.max(p.x,r.left-8))*Math.max(0,Math.min(p.y+h,r.top+r.height+8)-Math.max(p.y,r.top-8))
 return candidates.find(p=>protectedRects.every(r=>overlap(p,r)===0))??null
}
