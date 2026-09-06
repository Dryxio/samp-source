# Factory trial1: exact native bridge, misplaced outer failure result

Candidate79 vs original81. Prologue, all locals(EBP-4/-8/-C), ESI modelID, both calls and the complete slot2C bridge agree. Difference is the outer if(pGame&&lookup) falsepath currently jumps to the final result-local reload; original jumps to shared earlyfailure XOR EAX, thenrestoreframe. Candidate secondlookup-null path returns alreadyzeroEAX withoutXOR; original merges it with outerfailure and emitsXOR. This accounts for2bytes and moved positiveblock.

Single motivated C++ correction: keep resultinitialization andnativebridge unchanged, replace outerpositive conditional with earlyguard if(!pGame||!GetModelInfo(id)) returnNULL. Secondlookupnull earlyreturn nowshares the same explicitfailure exit. Source closure_model_instance_factory_trial2_proposed.cpp. No local/register volatility or assembly changed. No newdata/provider introduced. Rootprobe must compare all81 bytes and completebindings; not yetexact.
