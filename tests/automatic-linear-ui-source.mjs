import assert from 'node:assert/strict'
import fs from 'node:fs'
const source=fs.readFileSync(new URL('../source/src/app/bootstrap.ts',import.meta.url),'utf8')
for(const required of ['Global automatic alignment','Rigid · 6 parameters','Similarity · 7 parameters','Anisotropic scale · 9 parameters','Affine · 12 parameters','Full orientation','Normalized mutual information','Correlation ratio','Least squares','8× → 4× → 2× → 1×','Custom shrink factors','Auto for each level','Auto schedule'])assert(source.includes(required),`missing ${required}`)
assert(source.includes('runAutomaticLinear'))
assert(source.includes('shrinkFactorLevels'))
assert(!source.includes('samples across reference'))
console.log('automatic linear UI source checks passed')
