#include <spe.h>
__ev64_opaque__ plus(__ev64_opaque__ a,__ev64_opaque__ b){return __ev_addw(a,b);}
__ev64_opaque__ shift(__ev64_opaque__ a){return __ev_slwi(a,3);}
__ev64_opaque__ splat(void){return __ev_splati(-3);}
__ev64_opaque__ load(void *p){return __ev_ldd(p,1);}
void store(void *p,__ev64_opaque__ a){__ev_stdd(a,p,1);}
int all(__ev64_opaque__ a,__ev64_opaque__ b){return __ev_all_eq(a,b);}
__ev64_opaque__ selectv(__ev64_opaque__ a,__ev64_opaque__ b,__ev64_opaque__ c,__ev64_opaque__ d){return __ev_select_eq(a,b,c,d);}
