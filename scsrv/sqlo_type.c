#include <sqlora.h>
const char *
_get_data_type_str(int dtype)
{
  /* the constants are defined in ocidfn.h */
  switch (dtype) {

  case SQLOT_CHR: return "SQLOT_CHR:character string"; break;
  case SQLOT_NUM: return "SQLOT_NUM:oracle numeric"; break;
  case SQLOT_INT: return "SQLOT_INT:integer"; break;
  case SQLOT_FLT: return "SQLOT_FLT:floating point number"; break;
  case SQLOT_STR: return "SQLOT_STR:zero terminated string"; break;
  case SQLOT_VNU: return "SQLOT_VNU:num with preceding length byte"; break;
  case SQLOT_PDN: return "SQLOT_PDN:packed decimal numeric"; break;
  case SQLOT_LNG: return "SQLOT_LNG:long(string)"; break;
  case SQLOT_VCS: return "SQLOT_VCS:variable character string"; break;
  case SQLOT_NON: return "SQLOT_NON:Null/empty PCC Descriptor entry "; break;
  case SQLOT_RID: return "SQLOT_RID:rowid"; break;
  case SQLOT_DAT: return "SQLOT_DAT:date in oracle format"; break;
  case SQLOT_DATE: return "SQLOT_DATE:ANSI Date"; break;
  case SQLOT_TIME: return "SQLOT_TIME:Time"; break;
  case SQLOT_TIME_TZ: return "SQLOT_TIME_TZ:Time with timezone"; break;
  case SQLOT_TIMESTAMP: return "SQLOT_TIMESTAMP:Timestamp"; break;
  case SQLOT_TIMESTAMP_TZ: return "SQLOT_TIMESTAMP_TZ:Timestamp with timezone"; break;
  case SQLOT_TIMESTAMP_LTZ: return "SQLOT_TIMESTAMP_LTZ:Timestamp with local timezone"; break;
  case SQLOT_INTERVAL_YM: return "SQLOT_INTERVAL_YM:Interval year to month"; break;
  case SQLOT_INTERVAL_DS: return "SQLOT_INTERVAL_DS:Interval day to second"; break;
  case SQLOT_VBI: return "SQLOT_VBI:binary in VCS format"; break;
  case SQLOT_BIN: return "SQLOT_BIN:binary data(DTYBIN)"; break;
  case SQLOT_LBI: return "SQLOT_LBI:long binary"; break;
  case SQLOT_UIN: return "SQLOT_UIN:unsigned integer"; break;
  case SQLOT_SLS: return "SQLOT_SLS:dispay sign leading separate"; break;
  case SQLOT_LVC: return "SQLOT_LVC:longer longs (char)"; break;
  case SQLOT_LVB: return "SQLOT_LVB:longer longs (binary)"; break;
  case SQLOT_AFC: return "SQLOT_AFC:ansi fixed char"; break;
  case SQLOT_AVC: return "SQLOT_AVC:ansi var char"; break;
  case SQLOT_CUR: return "case SQLOT_CUR:cursor type"; break;
  case SQLOT_RDD: return "SQLOT_RDD:rowid descriptor"; break;
  case SQLOT_LAB: return "SQLOT_LAB:label type"; break;
  case SQLOT_OSL: return "SQLOT_OSL:oslabel type"; break;
  case SQLOT_NTY: return "SQLOT_NTY:named object type"; break;
  case SQLOT_REF: return "SQLOT_REF:ref type"; break;
  case SQLOT_CLOB: return "SQLOT_CLOB:character lob"; break;
  case SQLOT_BLOB: return "SQLOT_BLOB:binary lob"; break;
  case SQLOT_BFILEE: return "SQLOT_BFILEE:binary file lob"; break;
  case SQLOT_CFILEE: return "SQLOT_CFILEE:character file lob"; break;
  case SQLOT_RSET: return "SQLOT_RSET:result set type"; break;
  case SQLOT_NCO: return "SQLOT_NCO:named collection type"; break;
  case SQLOT_VST: return "SQLOT_VST:OCIString type"; break;
  case SQLOT_ODT: return "SQLOT_ODT:OCIDate type"; break;

  default:
    return "UNKNOWN";
    break;
  }
}
