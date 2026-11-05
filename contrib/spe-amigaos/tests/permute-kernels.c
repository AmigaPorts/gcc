typedef float v2sf __attribute__((vector_size(8)));
v2sf perm_00 (v2sf a, v2sf b)
{ return __builtin_shufflevector (a, b, 0, 0); }
v2sf perm_01 (v2sf a, v2sf b)
{ return __builtin_shufflevector (a, b, 0, 1); }
v2sf perm_02 (v2sf a, v2sf b)
{ return __builtin_shufflevector (a, b, 0, 2); }
v2sf perm_03 (v2sf a, v2sf b)
{ return __builtin_shufflevector (a, b, 0, 3); }
v2sf perm_10 (v2sf a, v2sf b)
{ return __builtin_shufflevector (a, b, 1, 0); }
v2sf perm_11 (v2sf a, v2sf b)
{ return __builtin_shufflevector (a, b, 1, 1); }
v2sf perm_12 (v2sf a, v2sf b)
{ return __builtin_shufflevector (a, b, 1, 2); }
v2sf perm_13 (v2sf a, v2sf b)
{ return __builtin_shufflevector (a, b, 1, 3); }
v2sf perm_20 (v2sf a, v2sf b)
{ return __builtin_shufflevector (a, b, 2, 0); }
v2sf perm_21 (v2sf a, v2sf b)
{ return __builtin_shufflevector (a, b, 2, 1); }
v2sf perm_22 (v2sf a, v2sf b)
{ return __builtin_shufflevector (a, b, 2, 2); }
v2sf perm_23 (v2sf a, v2sf b)
{ return __builtin_shufflevector (a, b, 2, 3); }
v2sf perm_30 (v2sf a, v2sf b)
{ return __builtin_shufflevector (a, b, 3, 0); }
v2sf perm_31 (v2sf a, v2sf b)
{ return __builtin_shufflevector (a, b, 3, 1); }
v2sf perm_32 (v2sf a, v2sf b)
{ return __builtin_shufflevector (a, b, 3, 2); }
v2sf perm_33 (v2sf a, v2sf b)
{ return __builtin_shufflevector (a, b, 3, 3); }
v2sf construct (float a, float b) { return (v2sf) { a, b }; }
v2sf splat (float a) { return (v2sf) { a, a }; }
float lane0 (v2sf x) { return x[0]; }
float lane1 (v2sf x) { return x[1]; }
float sum (v2sf x) { return x[0] + x[1]; }
float dot (v2sf a, v2sf b) { v2sf x = a * b; return x[0] + x[1]; }
