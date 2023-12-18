#pragma once
#include <3ds/types.h>
#include <3ds/os.h>
#include <assert.h>

// Result values
#define CFG_NOT_FOUND              MAKERESULT(RL_PERMANENT, RS_WRONGARG, RM_CONFIG, RD_NOT_FOUND)
#define CFG_NOT_IMPLEMENTED        MAKERESULT(RL_PERMANENT, RS_NOTSUPPORTED, RM_CONFIG, RD_NOT_IMPLEMENTED)
#define CFG_NOT_AUTHORIZED         MAKERESULT(RL_PERMANENT, RS_WRONGARG, RM_CONFIG, RD_NOT_AUTHORIZED)
//#define CFG_INVALID_SELECTION      MAKERESULT(RL_STATUS, RS_WRONGARG, RM_CONFIG, RD_INVALID_SELECTION)
#define CFG_INVALID_SIZE           MAKERESULT(RL_PERMANENT, RS_WRONGARG, RM_CONFIG, RD_INVALID_SIZE)
#define CFG_OUT_OF_MEMORY          MAKERESULT(RL_STATUS, RS_OUTOFRESOURCE, RM_CONFIG, RD_OUT_OF_MEMORY)
#define CFG_ALREADY_EXISTS         MAKERESULT(RL_PERMANENT, RS_WRONGARG, RM_CONFIG, RD_ALREADY_EXISTS)

#define CFG_NOT_INITIALIZED        MAKERESULT(RL_PERMANENT, RS_INVALIDSTATE, RM_CONFIG, RD_NOT_INITIALIZED)

#define CFG_BAD_SIGNATURE          MAKERESULT(RL_PERMANENT, RS_INVALIDSTATE, RM_CONFIG, 1)
#define CFG_FAILED_SIGNATURE_CHECK MAKERESULT(RL_PERMANENT, RS_INVALIDSTATE, RM_CONFIG, 2)
#define CFG_BAD_MODEL_CHECK        MAKERESULT(RL_PERMANENT, RS_INVALIDSTATE, RM_CONFIG, 6)

// Result values, my additions edition:tm:
#define CFG_INTERNAL_RANGE         MAKERESULT(RL_FATAL, RS_INTERNAL, RM_CONFIG, RD_OUT_OF_RANGE)
#define CFG_CANCELED_RANGE         MAKERESULT(RL_FATAL, RS_CANCELED, RM_CONFIG, RD_OUT_OF_RANGE)

/// Configuration region values.
typedef enum
{
	CFG_REGION_JPN = 0, ///< Japan
	CFG_REGION_USA = 1, ///< USA
	CFG_REGION_EUR = 2, ///< Europe
	CFG_REGION_AUS = 3, ///< Australia
	CFG_REGION_CHN = 4, ///< China
	CFG_REGION_KOR = 5, ///< Korea
	CFG_REGION_TWN = 6, ///< Taiwan
} CFG_Region;

enum PACKED CFG_CountryCode {
	CFG_COUNTRY_JP = 1,   // Japan
	CFG_COUNTRY_AI = 8,   // Anguilla
	CFG_COUNTRY_AG = 9,   // Antigua and Barbuda
	CFG_COUNTRY_AR = 10,  // Argentina
	CFG_COUNTRY_AW = 11,  // Aruba
	CFG_COUNTRY_BS = 12,  // Bahamas
	CFG_COUNTRY_BB = 13,  // Barbados
	CFG_COUNTRY_BZ = 14,  // Belize
	CFG_COUNTRY_BO = 15,  // Bolivia
	CFG_COUNTRY_BR = 16,  // Brazil
	CFG_COUNTRY_VG = 17,  // British Virgin Islands
	CFG_COUNTRY_CA = 18,  // Canada
	CFG_COUNTRY_KY = 19,  // Cayman Islands
	CFG_COUNTRY_CL = 20,  // Chile
	CFG_COUNTRY_CO = 21,  // Colombia
	CFG_COUNTRY_CR = 22,  // Costa Rica
	CFG_COUNTRY_DM = 23,  // Dominica
	CFG_COUNTRY_DO = 24,  // Dominican Republic
	CFG_COUNTRY_EC = 25,  // Ecuador
	CFG_COUNTRY_SV = 26,  // El Salvador
	CFG_COUNTRY_GF = 27,  // French Guiana
	CFG_COUNTRY_GD = 28,  // Grenada
	CFG_COUNTRY_GP = 29,  // Guadeloupe
	CFG_COUNTRY_GT = 30,  // Guatemala
	CFG_COUNTRY_GY = 31,  // Guyana
	CFG_COUNTRY_HT = 32,  // Haiti
	CFG_COUNTRY_HN = 33,  // Honduras
	CFG_COUNTRY_JM = 34,  // Jamaica
	CFG_COUNTRY_MQ = 35,  // Martinique
	CFG_COUNTRY_MX = 36,  // Mexico
	CFG_COUNTRY_MS = 37,  // Montserrat
	CFG_COUNTRY_AN = 38,  // Netherlands Antilles
	CFG_COUNTRY_NI = 39,  // Nicaragua
	CFG_COUNTRY_PA = 40,  // Panama
	CFG_COUNTRY_PY = 41,  // Paraguay
	CFG_COUNTRY_PE = 42,  // Peru
	CFG_COUNTRY_KN = 43,  // Saint Kitts and Nevis
	CFG_COUNTRY_LC = 44,  // Saint Lucia
	CFG_COUNTRY_VC = 45,  // Saint Vincent and the Grenadines
	CFG_COUNTRY_SR = 46,  // Suriname
	CFG_COUNTRY_TT = 47,  // Trinidad and Tobago
	CFG_COUNTRY_TC = 48,  // Turks and Caicos Islands
	CFG_COUNTRY_US = 49,  // United States
	CFG_COUNTRY_UY = 50,  // Uruguay
	CFG_COUNTRY_VI = 51,  // US Virgin Islands
	CFG_COUNTRY_VE = 52,  // Venezuela
	CFG_COUNTRY_AL = 64,  // Albania
	CFG_COUNTRY_AU = 65,  // Australia
	CFG_COUNTRY_AT = 66,  // Austria
	CFG_COUNTRY_BE = 67,  // Belgium
	CFG_COUNTRY_BA = 68,  // Bosnia and Herzegovina
	CFG_COUNTRY_BW = 69,  // Botswana
	CFG_COUNTRY_BG = 70,  // Bulgaria
	CFG_COUNTRY_HR = 71,  // Croatia
	CFG_COUNTRY_CY = 72,  // Cyprus
	CFG_COUNTRY_CZ = 73,  // Czech Republic
	CFG_COUNTRY_DK = 74,  // Denmark
	CFG_COUNTRY_EE = 75,  // Estonia
	CFG_COUNTRY_FI = 76,  // Finland
	CFG_COUNTRY_FR = 77,  // France
	CFG_COUNTRY_DE = 78,  // Germany
	CFG_COUNTRY_GR = 79,  // Greece
	CFG_COUNTRY_HU = 80,  // Hungary
	CFG_COUNTRY_IS = 81,  // Iceland
	CFG_COUNTRY_IE = 82,  // Ireland
	CFG_COUNTRY_IT = 83,  // Italy
	CFG_COUNTRY_LV = 84,  // Latvia
	CFG_COUNTRY_LS = 85,  // Lesotho
	CFG_COUNTRY_LI = 86,  // Liechtenstein
	CFG_COUNTRY_LT = 87,  // Lithuania
	CFG_COUNTRY_LU = 88,  // Luxembourg
	CFG_COUNTRY_MK = 89,  // Macedonia
	CFG_COUNTRY_MT = 90,  // Malta
	CFG_COUNTRY_ME = 91,  // Montenegro
	CFG_COUNTRY_MZ = 92,  // Mozambique
	CFG_COUNTRY_NA = 93,  // Namibia
	CFG_COUNTRY_NL = 94,  // Netherlands
	CFG_COUNTRY_NZ = 95,  // New Zealand
	CFG_COUNTRY_NO = 96,  // Norway
	CFG_COUNTRY_PL = 97,  // Poland
	CFG_COUNTRY_PT = 98,  // Portugal
	CFG_COUNTRY_RO = 99,  // Romania
	CFG_COUNTRY_RU = 100, // Russia
	CFG_COUNTRY_RS = 101, // Serbia and Kosovo
	CFG_COUNTRY_SK = 102, // Slovakia
	CFG_COUNTRY_SI = 103, // Slovenia
	CFG_COUNTRY_ZA = 104, // South Africa
	CFG_COUNTRY_ES = 105, // Spain
	CFG_COUNTRY_SZ = 106, // Swaziland
	CFG_COUNTRY_SE = 107, // Sweden
	CFG_COUNTRY_CH = 108, // Switzerland
	CFG_COUNTRY_TR = 109, // Turkey
	CFG_COUNTRY_GB = 110, // United Kingdom
	CFG_COUNTRY_ZM = 111, // Zambia
	CFG_COUNTRY_ZW = 112, // Zimbabwe
	CFG_COUNTRY_AZ = 113, // Azerbaijan
	CFG_COUNTRY_MR = 114, // Mauritania
	CFG_COUNTRY_ML = 115, // Mali
	CFG_COUNTRY_NE = 116, // Niger
	CFG_COUNTRY_TD = 117, // Chad
	CFG_COUNTRY_SD = 118, // Sudan
	CFG_COUNTRY_ER = 119, // Eritrea
	CFG_COUNTRY_DJ = 120, // Djibouti
	CFG_COUNTRY_SO = 121, // Somalia
	CFG_COUNTRY_AD = 122, // Andorra
	CFG_COUNTRY_GI = 123, // Gibraltar
	CFG_COUNTRY_GG = 124, // Guernsey
	CFG_COUNTRY_IM = 125, // Isle of Man
	CFG_COUNTRY_JE = 126, // Jersey
	CFG_COUNTRY_MC = 127, // Monaco
	CFG_COUNTRY_TW = 128, // Taiwan
	CFG_COUNTRY_KR = 136, // South Korea
	CFG_COUNTRY_HK = 144, // Hong Kong
	CFG_COUNTRY_MO = 145, // Macau
	CFG_COUNTRY_ID = 152, // Indonesia
	CFG_COUNTRY_SG = 153, // Singapore
	CFG_COUNTRY_TH = 154, // Thailand
	CFG_COUNTRY_PH = 155, // Philippines
	CFG_COUNTRY_MY = 156, // Malaysia
	CFG_COUNTRY_CN = 160, // China
	CFG_COUNTRY_AE = 168, // United Arab Emirates
	CFG_COUNTRY_IN = 169, // India
	CFG_COUNTRY_EG = 170, // Egypt
	CFG_COUNTRY_OM = 171, // Oman
	CFG_COUNTRY_QA = 172, // Qatar
	CFG_COUNTRY_KW = 173, // Kuwait
	CFG_COUNTRY_SA = 174, // Saudi Arabia
	CFG_COUNTRY_SY = 175, // Syria
	CFG_COUNTRY_BH = 176, // Bahrain
	CFG_COUNTRY_JO = 177, // Jordan
	CFG_COUNTRY_SM = 184, // San Marino
	CFG_COUNTRY_VA = 185, // Vatican City
	CFG_COUNTRY_BM = 186, // Bermuda
	CFG_COUNTRY_MAX = 186,
	CFG_COUNTRY_INVALID = 255,
	CFG_COUNTRY_16BIT = 0xFFFF // dummy to make this 16 bit
};

static_assert(sizeof(CFG_CountryCode) == 2);

/// Configuration language values.
typedef enum
{
	CFG_LANGUAGE_JP = 0,  ///< Japanese
	CFG_LANGUAGE_EN = 1,  ///< English
	CFG_LANGUAGE_FR = 2,  ///< French
	CFG_LANGUAGE_DE = 3,  ///< German
	CFG_LANGUAGE_IT = 4,  ///< Italian
	CFG_LANGUAGE_ES = 5,  ///< Spanish
	CFG_LANGUAGE_ZH = 6,  ///< Simplified Chinese
	CFG_LANGUAGE_KO = 7,  ///< Korean
	CFG_LANGUAGE_NL = 8,  ///< Dutch
	CFG_LANGUAGE_PT = 9,  ///< Portugese
	CFG_LANGUAGE_RU = 10, ///< Russian
	CFG_LANGUAGE_TW = 11, ///< Traditional Chinese
} CFG_Language;

// Configuration system model values.
typedef PACKED enum
{
	CFG_MODEL_3DS    = 0, ///< Old 3DS (CTR)
	CFG_MODEL_3DSXL  = 1, ///< Old 3DS XL (SPR)
	CFG_MODEL_N3DS   = 2, ///< New 3DS (KTR)
	CFG_MODEL_2DS    = 3, ///< Old 2DS (FTR)
	CFG_MODEL_N3DSXL = 4, ///< New 3DS XL (RED)
	CFG_MODEL_N2DSXL = 5, ///< New 2DS XL (JAN)
} CFG_SystemModel;

static_assert(sizeof(CFG_SystemModel) == 1);

typedef enum PACKED CFG_BlkFlags {
	BLK_USER_WRITE_PERM = 0x1, // although, this isn't in normal cases allowed, but write functions check this bit as if its real, so I take it to assumption
	BLK_USER_READ_PERM = 0x2,

	BLK_SYSTEM_WRITE_PERM = 0x4,
	BLK_SYSTEM_READ_PERM = 0x8,

	BLK_RO_SYSTEM = BLK_SYSTEM_READ_PERM,
	BLK_RO_ANY = BLK_RO_SYSTEM | BLK_USER_READ_PERM,

	BLK_RW_SYSTEM = BLK_RO_SYSTEM | BLK_SYSTEM_WRITE_PERM,
	BLK_RW_ANY = BLK_RO_ANY | BLK_SYSTEM_WRITE_PERM, // user never writes

	BLK_SYSTEM_ALL_PERM = BLK_SYSTEM_READ_PERM | BLK_SYSTEM_WRITE_PERM,
	BLK_USER_ALL_PERM = BLK_USER_READ_PERM | BLK_USER_WRITE_PERM,

	BLK_READ_PERM_BITMASK = BLK_SYSTEM_READ_PERM | BLK_USER_READ_PERM,
	BLK_WRITE_PERM_BITMASK =  BLK_SYSTEM_WRITE_PERM | BLK_USER_WRITE_PERM, // mask used on write function when checking bits, even tho user can never write

	BLK_ALL_PERM_BITMASK = BLK_SYSTEM_ALL_PERM | BLK_USER_ALL_PERM,

	BLK_MAX_16BIT = 0xFFFF // force 16bit
} CFG_BlkFlags;

static_assert(sizeof(CFG_BlkFlags) == 2);
