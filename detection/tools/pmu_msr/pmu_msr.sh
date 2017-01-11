#! /bin/bash

#Clear fixed MSRs
wrmsr -p 0 0x309 0x0
wrmsr -p 0 0x30A 0x0
wrmsr -p 0 0x30B 0x0

#Clear general-purpose MSR
wrmsr -p 0 0xC1 0x0
wrmsr -p 0 0xC2 0x0
wrmsr -p 0 0xC3 0x0
wrmsr -p 0 0xC4 0x0

#Start counting by writing to IA32_FIXED_CTR_CTRL
wrmsr -p 0 0x38D 0x222

#Start counting by writing to IA32_PERFEVTSEL
wrmsr -p 0 0x186 0x51c000

#./pmu_msr

#Stop counting on general-purpose MSRs
wrmsr -p 0 0x186 0x11c000

#Read fixed MSRs:
echo -n 'Inst_Retired.Any = '
rdmsr -u -p 0 0x309
echo -n 'CPU_CLK_Unhalted.Core = '
rdmsr -u -p 0 0x30A
echo -n 'CPU_CLK_Unhalted.Ref = '
rdmsr -u -p 0 0x30B

#Read general-purpose MSRs:
echo -n 'remote caches = '
rdmsr -u -p 0 0xC1
