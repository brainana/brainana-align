/* Fixed, non-sensitive analytic inputs. x is the fastest varying index. */
static float phantom_value(int x,int y,int z) {
  return 1+80*exp(-((x-3.)*(x-3.)+(y-4.)*(y-4.)+(z-4.)*(z-4.))/8.)
          +15*exp(-((x-6.)*(x-6.)+(y-2.)*(y-2.)+(z-3.)*(z-3.))/3.);
}
