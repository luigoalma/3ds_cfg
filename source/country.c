#include <3ds/types.h>
#include <cfg.h>

typedef union {
	char str[2];
	u16 str16;
} IsoStr;

#define INVALID_COUNTRY  ((IsoStr){.str16 = 0})

#define CFG_COUNTRY_JP_STR  ((IsoStr){.str = {'J','P'}})
#define CFG_COUNTRY_AI_STR  ((IsoStr){.str = {'A','I'}})
#define CFG_COUNTRY_AG_STR  ((IsoStr){.str = {'A','G'}})
#define CFG_COUNTRY_AR_STR  ((IsoStr){.str = {'A','R'}})
#define CFG_COUNTRY_AW_STR  ((IsoStr){.str = {'A','W'}})
#define CFG_COUNTRY_BS_STR  ((IsoStr){.str = {'B','S'}})
#define CFG_COUNTRY_BB_STR  ((IsoStr){.str = {'B','B'}})
#define CFG_COUNTRY_BZ_STR  ((IsoStr){.str = {'B','Z'}})
#define CFG_COUNTRY_BO_STR  ((IsoStr){.str = {'B','O'}})
#define CFG_COUNTRY_BR_STR  ((IsoStr){.str = {'B','R'}})
#define CFG_COUNTRY_VG_STR  ((IsoStr){.str = {'V','G'}})
#define CFG_COUNTRY_CA_STR  ((IsoStr){.str = {'C','A'}})
#define CFG_COUNTRY_KY_STR  ((IsoStr){.str = {'K','Y'}})
#define CFG_COUNTRY_CL_STR  ((IsoStr){.str = {'C','L'}})
#define CFG_COUNTRY_CO_STR  ((IsoStr){.str = {'C','O'}})
#define CFG_COUNTRY_CR_STR  ((IsoStr){.str = {'C','R'}})
#define CFG_COUNTRY_DM_STR  ((IsoStr){.str = {'D','M'}})
#define CFG_COUNTRY_DO_STR  ((IsoStr){.str = {'D','O'}})
#define CFG_COUNTRY_EC_STR  ((IsoStr){.str = {'E','C'}})
#define CFG_COUNTRY_SV_STR  ((IsoStr){.str = {'S','V'}})
#define CFG_COUNTRY_GF_STR  ((IsoStr){.str = {'G','F'}})
#define CFG_COUNTRY_GD_STR  ((IsoStr){.str = {'G','D'}})
#define CFG_COUNTRY_GP_STR  ((IsoStr){.str = {'G','P'}})
#define CFG_COUNTRY_GT_STR  ((IsoStr){.str = {'G','T'}})
#define CFG_COUNTRY_GY_STR  ((IsoStr){.str = {'G','Y'}})
#define CFG_COUNTRY_HT_STR  ((IsoStr){.str = {'H','T'}})
#define CFG_COUNTRY_HN_STR  ((IsoStr){.str = {'H','N'}})
#define CFG_COUNTRY_JM_STR  ((IsoStr){.str = {'J','M'}})
#define CFG_COUNTRY_MQ_STR  ((IsoStr){.str = {'M','Q'}})
#define CFG_COUNTRY_MX_STR  ((IsoStr){.str = {'M','X'}})
#define CFG_COUNTRY_MS_STR  ((IsoStr){.str = {'M','S'}})
#define CFG_COUNTRY_AN_STR  ((IsoStr){.str = {'A','N'}})
#define CFG_COUNTRY_NI_STR  ((IsoStr){.str = {'N','I'}})
#define CFG_COUNTRY_PA_STR  ((IsoStr){.str = {'P','A'}})
#define CFG_COUNTRY_PY_STR  ((IsoStr){.str = {'P','Y'}})
#define CFG_COUNTRY_PE_STR  ((IsoStr){.str = {'P','E'}})
#define CFG_COUNTRY_KN_STR  ((IsoStr){.str = {'K','N'}})
#define CFG_COUNTRY_LC_STR  ((IsoStr){.str = {'L','C'}})
#define CFG_COUNTRY_VC_STR  ((IsoStr){.str = {'V','C'}})
#define CFG_COUNTRY_SR_STR  ((IsoStr){.str = {'S','R'}})
#define CFG_COUNTRY_TT_STR  ((IsoStr){.str = {'T','T'}})
#define CFG_COUNTRY_TC_STR  ((IsoStr){.str = {'T','C'}})
#define CFG_COUNTRY_US_STR  ((IsoStr){.str = {'U','S'}})
#define CFG_COUNTRY_UY_STR  ((IsoStr){.str = {'U','Y'}})
#define CFG_COUNTRY_VI_STR  ((IsoStr){.str = {'V','I'}})
#define CFG_COUNTRY_VE_STR  ((IsoStr){.str = {'V','E'}})
#define CFG_COUNTRY_AL_STR  ((IsoStr){.str = {'A','L'}})
#define CFG_COUNTRY_AU_STR  ((IsoStr){.str = {'A','U'}})
#define CFG_COUNTRY_AT_STR  ((IsoStr){.str = {'A','T'}})
#define CFG_COUNTRY_BE_STR  ((IsoStr){.str = {'B','E'}})
#define CFG_COUNTRY_BA_STR  ((IsoStr){.str = {'B','A'}})
#define CFG_COUNTRY_BW_STR  ((IsoStr){.str = {'B','W'}})
#define CFG_COUNTRY_BG_STR  ((IsoStr){.str = {'B','G'}})
#define CFG_COUNTRY_HR_STR  ((IsoStr){.str = {'H','R'}})
#define CFG_COUNTRY_CY_STR  ((IsoStr){.str = {'C','Y'}})
#define CFG_COUNTRY_CZ_STR  ((IsoStr){.str = {'C','Z'}})
#define CFG_COUNTRY_DK_STR  ((IsoStr){.str = {'D','K'}})
#define CFG_COUNTRY_EE_STR  ((IsoStr){.str = {'E','E'}})
#define CFG_COUNTRY_FI_STR  ((IsoStr){.str = {'F','I'}})
#define CFG_COUNTRY_FR_STR  ((IsoStr){.str = {'F','R'}})
#define CFG_COUNTRY_DE_STR  ((IsoStr){.str = {'D','E'}})
#define CFG_COUNTRY_GR_STR  ((IsoStr){.str = {'G','R'}})
#define CFG_COUNTRY_HU_STR  ((IsoStr){.str = {'H','U'}})
#define CFG_COUNTRY_IS_STR  ((IsoStr){.str = {'I','S'}})
#define CFG_COUNTRY_IE_STR  ((IsoStr){.str = {'I','E'}})
#define CFG_COUNTRY_IT_STR  ((IsoStr){.str = {'I','T'}})
#define CFG_COUNTRY_LV_STR  ((IsoStr){.str = {'L','V'}})
#define CFG_COUNTRY_LS_STR  ((IsoStr){.str = {'L','S'}})
#define CFG_COUNTRY_LI_STR  ((IsoStr){.str = {'L','I'}})
#define CFG_COUNTRY_LT_STR  ((IsoStr){.str = {'L','T'}})
#define CFG_COUNTRY_LU_STR  ((IsoStr){.str = {'L','U'}})
#define CFG_COUNTRY_MK_STR  ((IsoStr){.str = {'M','K'}})
#define CFG_COUNTRY_MT_STR  ((IsoStr){.str = {'M','T'}})
#define CFG_COUNTRY_ME_STR  ((IsoStr){.str = {'M','E'}})
#define CFG_COUNTRY_MZ_STR  ((IsoStr){.str = {'M','Z'}})
#define CFG_COUNTRY_NA_STR  ((IsoStr){.str = {'N','A'}})
#define CFG_COUNTRY_NL_STR  ((IsoStr){.str = {'N','L'}})
#define CFG_COUNTRY_NZ_STR  ((IsoStr){.str = {'N','Z'}})
#define CFG_COUNTRY_NO_STR  ((IsoStr){.str = {'N','O'}})
#define CFG_COUNTRY_PL_STR  ((IsoStr){.str = {'P','L'}})
#define CFG_COUNTRY_PT_STR  ((IsoStr){.str = {'P','T'}})
#define CFG_COUNTRY_RO_STR  ((IsoStr){.str = {'R','O'}})
#define CFG_COUNTRY_RU_STR  ((IsoStr){.str = {'R','U'}})
#define CFG_COUNTRY_RS_STR  ((IsoStr){.str = {'R','S'}})
#define CFG_COUNTRY_SK_STR  ((IsoStr){.str = {'S','K'}})
#define CFG_COUNTRY_SI_STR  ((IsoStr){.str = {'S','I'}})
#define CFG_COUNTRY_ZA_STR  ((IsoStr){.str = {'Z','A'}})
#define CFG_COUNTRY_ES_STR  ((IsoStr){.str = {'E','S'}})
#define CFG_COUNTRY_SZ_STR  ((IsoStr){.str = {'S','Z'}})
#define CFG_COUNTRY_SE_STR  ((IsoStr){.str = {'S','E'}})
#define CFG_COUNTRY_CH_STR  ((IsoStr){.str = {'C','H'}})
#define CFG_COUNTRY_TR_STR  ((IsoStr){.str = {'T','R'}})
#define CFG_COUNTRY_GB_STR  ((IsoStr){.str = {'G','B'}})
#define CFG_COUNTRY_ZM_STR  ((IsoStr){.str = {'Z','M'}})
#define CFG_COUNTRY_ZW_STR  ((IsoStr){.str = {'Z','W'}})
#define CFG_COUNTRY_AZ_STR  ((IsoStr){.str = {'A','Z'}})
#define CFG_COUNTRY_MR_STR  ((IsoStr){.str = {'M','R'}})
#define CFG_COUNTRY_ML_STR  ((IsoStr){.str = {'M','L'}})
#define CFG_COUNTRY_NE_STR  ((IsoStr){.str = {'N','E'}})
#define CFG_COUNTRY_TD_STR  ((IsoStr){.str = {'T','D'}})
#define CFG_COUNTRY_SD_STR  ((IsoStr){.str = {'S','D'}})
#define CFG_COUNTRY_ER_STR  ((IsoStr){.str = {'E','R'}})
#define CFG_COUNTRY_DJ_STR  ((IsoStr){.str = {'D','J'}})
#define CFG_COUNTRY_SO_STR  ((IsoStr){.str = {'S','O'}})
#define CFG_COUNTRY_AD_STR  ((IsoStr){.str = {'A','D'}})
#define CFG_COUNTRY_GI_STR  ((IsoStr){.str = {'G','I'}})
#define CFG_COUNTRY_GG_STR  ((IsoStr){.str = {'G','G'}})
#define CFG_COUNTRY_IM_STR  ((IsoStr){.str = {'I','M'}})
#define CFG_COUNTRY_JE_STR  ((IsoStr){.str = {'J','E'}})
#define CFG_COUNTRY_MC_STR  ((IsoStr){.str = {'M','C'}})
#define CFG_COUNTRY_TW_STR  ((IsoStr){.str = {'T','W'}})
#define CFG_COUNTRY_KR_STR  ((IsoStr){.str = {'K','R'}})
#define CFG_COUNTRY_HK_STR  ((IsoStr){.str = {'H','K'}})
#define CFG_COUNTRY_MO_STR  ((IsoStr){.str = {'M','O'}})
#define CFG_COUNTRY_ID_STR  ((IsoStr){.str = {'I','D'}})
#define CFG_COUNTRY_SG_STR  ((IsoStr){.str = {'S','G'}})
#define CFG_COUNTRY_TH_STR  ((IsoStr){.str = {'T','H'}})
#define CFG_COUNTRY_PH_STR  ((IsoStr){.str = {'P','H'}})
#define CFG_COUNTRY_MY_STR  ((IsoStr){.str = {'M','Y'}})
#define CFG_COUNTRY_CN_STR  ((IsoStr){.str = {'C','N'}})
#define CFG_COUNTRY_AE_STR  ((IsoStr){.str = {'A','E'}})
#define CFG_COUNTRY_IN_STR  ((IsoStr){.str = {'I','N'}})
#define CFG_COUNTRY_EG_STR  ((IsoStr){.str = {'E','G'}})
#define CFG_COUNTRY_OM_STR  ((IsoStr){.str = {'O','M'}})
#define CFG_COUNTRY_QA_STR  ((IsoStr){.str = {'Q','A'}})
#define CFG_COUNTRY_KW_STR  ((IsoStr){.str = {'K','W'}})
#define CFG_COUNTRY_SA_STR  ((IsoStr){.str = {'S','A'}})
#define CFG_COUNTRY_SY_STR  ((IsoStr){.str = {'S','Y'}})
#define CFG_COUNTRY_BH_STR  ((IsoStr){.str = {'B','H'}})
#define CFG_COUNTRY_JO_STR  ((IsoStr){.str = {'J','O'}})
#define CFG_COUNTRY_SM_STR  ((IsoStr){.str = {'S','M'}})
#define CFG_COUNTRY_VA_STR  ((IsoStr){.str = {'V','A'}})
#define CFG_COUNTRY_BM_STR  ((IsoStr){.str = {'B','M'}})

const IsoStr CountryIsoStringTable[187] = {
	INVALID_COUNTRY,    // 0
	CFG_COUNTRY_JP_STR, // 1
	INVALID_COUNTRY,    // 2
	INVALID_COUNTRY,    // 3
	INVALID_COUNTRY,    // 4
	INVALID_COUNTRY,    // 5
	INVALID_COUNTRY,    // 6
	INVALID_COUNTRY,    // 7
	CFG_COUNTRY_AI_STR, // 8
	CFG_COUNTRY_AG_STR, // 9
	CFG_COUNTRY_AR_STR, // 10
	CFG_COUNTRY_AW_STR, // 11
	CFG_COUNTRY_BS_STR, // 12
	CFG_COUNTRY_BB_STR, // 13
	CFG_COUNTRY_BZ_STR, // 14
	CFG_COUNTRY_BO_STR, // 15
	CFG_COUNTRY_BR_STR, // 16
	CFG_COUNTRY_VG_STR, // 17
	CFG_COUNTRY_CA_STR, // 18
	CFG_COUNTRY_KY_STR, // 19
	CFG_COUNTRY_CL_STR, // 20
	CFG_COUNTRY_CO_STR, // 21
	CFG_COUNTRY_CR_STR, // 22
	CFG_COUNTRY_DM_STR, // 23
	CFG_COUNTRY_DO_STR, // 24
	CFG_COUNTRY_EC_STR, // 25
	CFG_COUNTRY_SV_STR, // 26
	CFG_COUNTRY_GF_STR, // 27
	CFG_COUNTRY_GD_STR, // 28
	CFG_COUNTRY_GP_STR, // 29
	CFG_COUNTRY_GT_STR, // 30
	CFG_COUNTRY_GY_STR, // 31
	CFG_COUNTRY_HT_STR, // 32
	CFG_COUNTRY_HN_STR, // 33
	CFG_COUNTRY_JM_STR, // 34
	CFG_COUNTRY_MQ_STR, // 35
	CFG_COUNTRY_MX_STR, // 36
	CFG_COUNTRY_MS_STR, // 37
	CFG_COUNTRY_AN_STR, // 38
	CFG_COUNTRY_NI_STR, // 39
	CFG_COUNTRY_PA_STR, // 40
	CFG_COUNTRY_PY_STR, // 41
	CFG_COUNTRY_PE_STR, // 42
	CFG_COUNTRY_KN_STR, // 43
	CFG_COUNTRY_LC_STR, // 44
	CFG_COUNTRY_VC_STR, // 45
	CFG_COUNTRY_SR_STR, // 46
	CFG_COUNTRY_TT_STR, // 47
	CFG_COUNTRY_TC_STR, // 48
	CFG_COUNTRY_US_STR, // 49
	CFG_COUNTRY_UY_STR, // 50
	CFG_COUNTRY_VI_STR, // 51
	CFG_COUNTRY_VE_STR, // 52
	INVALID_COUNTRY,    // 53
	INVALID_COUNTRY,    // 54
	INVALID_COUNTRY,    // 55
	INVALID_COUNTRY,    // 56
	INVALID_COUNTRY,    // 57
	INVALID_COUNTRY,    // 58
	INVALID_COUNTRY,    // 59
	INVALID_COUNTRY,    // 60
	INVALID_COUNTRY,    // 61
	INVALID_COUNTRY,    // 62
	INVALID_COUNTRY,    // 63
	CFG_COUNTRY_AL_STR, // 64
	CFG_COUNTRY_AU_STR, // 65
	CFG_COUNTRY_AT_STR, // 66
	CFG_COUNTRY_BE_STR, // 67
	CFG_COUNTRY_BA_STR, // 68
	CFG_COUNTRY_BW_STR, // 69
	CFG_COUNTRY_BG_STR, // 70
	CFG_COUNTRY_HR_STR, // 71
	CFG_COUNTRY_CY_STR, // 72
	CFG_COUNTRY_CZ_STR, // 73
	CFG_COUNTRY_DK_STR, // 74
	CFG_COUNTRY_EE_STR, // 75
	CFG_COUNTRY_FI_STR, // 76
	CFG_COUNTRY_FR_STR, // 77
	CFG_COUNTRY_DE_STR, // 78
	CFG_COUNTRY_GR_STR, // 79
	CFG_COUNTRY_HU_STR, // 80
	CFG_COUNTRY_IS_STR, // 81
	CFG_COUNTRY_IE_STR, // 82
	CFG_COUNTRY_IT_STR, // 83
	CFG_COUNTRY_LV_STR, // 84
	CFG_COUNTRY_LS_STR, // 85
	CFG_COUNTRY_LI_STR, // 86
	CFG_COUNTRY_LT_STR, // 87
	CFG_COUNTRY_LU_STR, // 88
	CFG_COUNTRY_MK_STR, // 89
	CFG_COUNTRY_MT_STR, // 90
	CFG_COUNTRY_ME_STR, // 91
	CFG_COUNTRY_MZ_STR, // 92
	CFG_COUNTRY_NA_STR, // 93
	CFG_COUNTRY_NL_STR, // 94
	CFG_COUNTRY_NZ_STR, // 95
	CFG_COUNTRY_NO_STR, // 96
	CFG_COUNTRY_PL_STR, // 97
	CFG_COUNTRY_PT_STR, // 98
	CFG_COUNTRY_RO_STR, // 99
	CFG_COUNTRY_RU_STR, // 100
	CFG_COUNTRY_RS_STR, // 101
	CFG_COUNTRY_SK_STR, // 102
	CFG_COUNTRY_SI_STR, // 103
	CFG_COUNTRY_ZA_STR, // 104
	CFG_COUNTRY_ES_STR, // 105
	CFG_COUNTRY_SZ_STR, // 106
	CFG_COUNTRY_SE_STR, // 107
	CFG_COUNTRY_CH_STR, // 108
	CFG_COUNTRY_TR_STR, // 109
	CFG_COUNTRY_GB_STR, // 110
	CFG_COUNTRY_ZM_STR, // 111
	CFG_COUNTRY_ZW_STR, // 112
	CFG_COUNTRY_AZ_STR, // 113
	CFG_COUNTRY_MR_STR, // 114
	CFG_COUNTRY_ML_STR, // 115
	CFG_COUNTRY_NE_STR, // 116
	CFG_COUNTRY_TD_STR, // 117
	CFG_COUNTRY_SD_STR, // 118
	CFG_COUNTRY_ER_STR, // 119
	CFG_COUNTRY_DJ_STR, // 120
	CFG_COUNTRY_SO_STR, // 121
	CFG_COUNTRY_AD_STR, // 122
	CFG_COUNTRY_GI_STR, // 123
	CFG_COUNTRY_GG_STR, // 124
	CFG_COUNTRY_IM_STR, // 125
	CFG_COUNTRY_JE_STR, // 126
	CFG_COUNTRY_MC_STR, // 127
	CFG_COUNTRY_TW_STR, // 128
	INVALID_COUNTRY,    // 129
	INVALID_COUNTRY,    // 130
	INVALID_COUNTRY,    // 131
	INVALID_COUNTRY,    // 132
	INVALID_COUNTRY,    // 133
	INVALID_COUNTRY,    // 134
	INVALID_COUNTRY,    // 135
	CFG_COUNTRY_KR_STR, // 136
	INVALID_COUNTRY,    // 137
	INVALID_COUNTRY,    // 138
	INVALID_COUNTRY,    // 139
	INVALID_COUNTRY,    // 140
	INVALID_COUNTRY,    // 141
	INVALID_COUNTRY,    // 142
	INVALID_COUNTRY,    // 143
	CFG_COUNTRY_HK_STR, // 144
	CFG_COUNTRY_MO_STR, // 145
	INVALID_COUNTRY,    // 146
	INVALID_COUNTRY,    // 147
	INVALID_COUNTRY,    // 148
	INVALID_COUNTRY,    // 149
	INVALID_COUNTRY,    // 150
	INVALID_COUNTRY,    // 151
	CFG_COUNTRY_ID_STR, // 152
	CFG_COUNTRY_SG_STR, // 153
	CFG_COUNTRY_TH_STR, // 154
	CFG_COUNTRY_PH_STR, // 155
	CFG_COUNTRY_MY_STR, // 156
	INVALID_COUNTRY,    // 157
	INVALID_COUNTRY,    // 158
	INVALID_COUNTRY,    // 159
	CFG_COUNTRY_CN_STR, // 160
	INVALID_COUNTRY,    // 161
	INVALID_COUNTRY,    // 162
	INVALID_COUNTRY,    // 163
	INVALID_COUNTRY,    // 164
	INVALID_COUNTRY,    // 165
	INVALID_COUNTRY,    // 166
	INVALID_COUNTRY,    // 167
	CFG_COUNTRY_AE_STR, // 168
	CFG_COUNTRY_IN_STR, // 169
	CFG_COUNTRY_EG_STR, // 170
	CFG_COUNTRY_OM_STR, // 171
	CFG_COUNTRY_QA_STR, // 172
	CFG_COUNTRY_KW_STR, // 173
	CFG_COUNTRY_SA_STR, // 174
	CFG_COUNTRY_SY_STR, // 175
	CFG_COUNTRY_BH_STR, // 176
	CFG_COUNTRY_JO_STR, // 177
	INVALID_COUNTRY,    // 178
	INVALID_COUNTRY,    // 179
	INVALID_COUNTRY,    // 180
	INVALID_COUNTRY,    // 181
	INVALID_COUNTRY,    // 182
	INVALID_COUNTRY,    // 183
	CFG_COUNTRY_SM_STR, // 184
	CFG_COUNTRY_VA_STR, // 185
	CFG_COUNTRY_BM_STR  // 186
};

Result CountryStrToEnum(const char* str, CFG_CountryCode* code) {
	if(str[2] != 0) {
		*code = CFG_COUNTRY_INVALID;
		return CFG_NOT_FOUND;
	}
	IsoStr _str = (IsoStr){.str = {str[0], str[1]}};
	for(int i = 0; i <= CFG_COUNTRY_MAX; ++i) {
		if(CountryIsoStringTable[i].str16 == _str.str16) {
			*code = i;
			return 0;
		}
	}
	*code = CFG_COUNTRY_INVALID;
	return CFG_NOT_FOUND;
}

Result CountryEnumToStr(CFG_CountryCode code, char* str) {
	if(code > CFG_COUNTRY_MAX) {
		str[0] = 0;
		str[1] = 0;
		str[2] = 0;
		return CFG_NOT_FOUND;
	}
	const IsoStr* _str = &CountryIsoStringTable[code];
	str[0] = _str->str[0];
	str[1] = _str->str[1];
	str[2] = 0;
	return (_str->str[0] == 0 || _str->str[1] == 0) ? CFG_NOT_FOUND : 0;
}
