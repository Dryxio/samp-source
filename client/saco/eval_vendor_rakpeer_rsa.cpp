#include <string.h>
// Real vendor RSA specialization used as RakPeer::rsacrypt (256-bit modulus).
// Explicit instantiation supplies actual algorithms, no synthetic call wrappers.
// BigTypes.h retains its original symbolic x86 implementation where USEASSEMBLY
// selects it; no instructions or literal bytes have been reconstructed by copying.
#include "../raknet/RSACrypt.h"

template class big::RSACrypt<big::u256>;
template void big::RSACrypt<big::u256>::setPrivateKey<big::u128>(big::u128&, big::u128&);

typedef char R5RsaWord[(sizeof(big::u32)==4)?1:-1];
typedef char R5RsaModulus[(sizeof(big::u256)==32)?1:-1];
typedef char R5RsaFactor[(sizeof(big::u128)==16)?1:-1];
typedef char R5RsaComplete[(sizeof(big::RSACrypt<big::u256>)==305)?1:-1];
