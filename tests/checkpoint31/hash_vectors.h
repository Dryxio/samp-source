// Expected digests from Python hashlib for bytes i & 255; padding/block boundaries.
struct HashVector {unsigned length;const char* md5;const char* sha1;};
static const HashVector hashVectors[]={
    {55,"6912ee65fff2d9f9ce2508cddf8bcda0","8ae2d46729cfe68ff927af5eec9c7d1b66d65ac2"},
    {56,"51fdd1acda72405dfdfa03fcb85896d7","636e2ec698dac903498e648bd2f3af641d3c88cb"},
    {63,"48a6295221902e8e0938f773a7185e72","6d942da0c4392b123528f2905c713a3ce28364bd"},
    {64,"b2d3f56bc197fd985d5965079b5e7148","c6138d514ffa2135bfce0ed0b8fac65669917ec7"},
    {65,"8bd7053801c768420faf816fadba971c","69bd728ad6e13cd76ff19751fde427b00e395746"},
    {127,"8402b21e7bc7906493bae0dac017f1f9","89d7312a903f65cd2b3e34a975e55dbea9033353"},
    {128,"37eff01866ba3f538421b30b7cbefcac","e6434bc401f98603d7eda504790c98c67385d535"},
    {129,"46f986692847558fc38b0cece591c20f","3352e41cc30b40ae80108970492b21014049e625"},
    {1000,"cbecbdb0fdd5cec1e242493b6008cc79","af0b191c2de46fe13fe0908f5a6a4e90e0cafc46"},
};
