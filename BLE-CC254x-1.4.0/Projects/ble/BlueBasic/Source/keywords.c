#include <stdio.h>
#include <stdlib.h>
#include <string.h>

struct keyword
{
  char* keyword;
  char* token;
} keywords[] =
{
  { "LIST", "KW_LIST" },
  { "NEW", "KW_NEW" },
  { "RUN", "KW_RUN" },
  { "NEXT", "KW_NEXT" },
  { "IF", "KW_IF" },
  { "ELSE", "KW_ELSE" },
  { "ELIF", "KW_ELIF" },
  { "GOTO", "KW_GOTO" },
  { "GOSUB", "KW_GOSUB" },
  { "RETURN", "KW_RETURN" },
  { "REM", "KW_REM" },
  { "//", "KW_SLASHSLASH" },
  { "FOR", "KW_FOR" },
  { "TO", "ST_TO" },
  { "STEP", "ST_STEP" },
  { "PRINT", "KW_PRINT" },
  { "REBOOT", "KW_REBOOT" },
  { "MEM", "KW_MEM" },
  { "DIM", "KW_DIM" },
  { "END", "KW_END" },
  { "TIMER", "KW_TIMER" },
  { "REPEAT", "TI_REPEAT" },
  { "STOP", "TI_STOP" },
  { "DELAY", "KW_DELAY" },
  { "ABS", "FUNC_ABS" },
  { "LEN", "FUNC_LEN" },
  { "RND", "FUNC_RND" },
  { "MILLIS", "FUNC_MILLIS" },
  { "BATTERY", "FUNC_BATTERY" },
  { "AUTORUN", "KW_AUTORUN" },
  { ">=", "OP_GE" },
  { "<>", "OP_NE" },
  { ">", "OP_GT" },
  { "==", "OP_EQEQ" },
  { "=", "OP_EQ" },
  { "<=", "OP_LE" },
  { "<", "OP_LT" },
  { "!=", "OP_NE_BANG" },
  { "0X", "FUNC_HEX" },
  { "+", "OP_ADD" },
  { "-", "OP_SUB" },
  { "*", "OP_MUL" },
  { "/", "OP_DIV" },
  { "%", "OP_REM" },
  { "&", "OP_AND" },
  { "|", "OP_OR" },
  { "^", "OP_XOR" },
  { "<<", "OP_LSHIFT" },
  { ">>", "OP_RSHIFT" },
  { "PINMODE", "KW_PINMODE" },
  { "SERIAL", "KW_SERIAL" },
  { "INTERRUPT", "KW_INTERRUPT" },
  { "ATTACH", "IN_ATTACH" },
  { "DETACH", "IN_DETACH" },
  { "PULLUP", "PM_PULLUP" },
  { "PULLDOWN", "PM_PULLDOWN" },
  { "INPUT", "PM_INPUT" },
  { "ADC", "PM_ADC" },
  { "OUTPUT", "PM_OUTPUT" },
  { "RISING", "PM_RISING" },
  { "FALLING", "PM_FALLING" },
  { "P0", "KW_PIN_P0" },
  { "P1", "KW_PIN_P1" },
  { "P2", "KW_PIN_P2" },
  { "ANALOG", "KW_ANALOG" },
  { "CONFIG", "KW_CONFIG" },
  { "REFERENCE", "KW_CONSTANT,CO_REFERENCE" },
  { "RESOLUTION", "KW_CONSTANT,CO_RESOLUTION" },
  { "INTERNAL", "KW_CONSTANT,CO_INTERNAL" },
  { "EXTERNAL", "KW_CONSTANT,CO_EXTERNAL" },
  { "ONREAD", "BLE_ONREAD" },
  { "ONWRITE", "BLE_ONWRITE" },
  { "ONCONNECT", "BLE_ONCONNECT" },
  { "ONDISCOVER", "BLE_ONDISCOVER" },
  { "GATT", "KW_GATT" },
  { "SERVICE", "BLE_SERVICE" },
  { "CHARACTERISTIC", "BLE_CHARACTERISTIC" },
  { "READ", "KW_READ" },
  { "WRITENORSP", "BLE_WRITENORSP" },
  { "WRITE", "KW_WRITE" },
  { "AUTH", "BLE_AUTH" },
  { "NOTIFY", "BLE_NOTIFY" },
  { "INDICATE", "BLE_INDICATE" },
  { "ADVERT", "KW_ADVERT" },
  { "SCAN", "KW_SCAN" },
  { "GENERAL", "BLE_GENERAL" },
  { "LIMITED", "BLE_LIMITED" },
  { "MORE", "BLE_MORE" },
  { "NAME", "BLE_NAME" },
  { "CUSTOM", "BLE_CUSTOM" },
  { "BTPEEK", "BLE_FUNC_BTPEEK" },
  { "BTPOKE", "KW_BTPOKE" },
  { "SPI", "KW_SPI" },
  { "TRANSFER", "SPI_TRANSFER" },
  { "MSB", "SPI_MSB" },
  { "LSB", "SPI_LSB" },
  { "MASTER", "SPI_MASTER" },
  { "SLAVE", "SPI_SLAVE" },
  { "ACTIVE", "BLE_ACTIVE" },
  { "DUPLICATES", "BLE_DUPLICATES" },
  { "WIRE", "KW_WIRE" },
  { "TIMEOUT", "PM_TIMEOUT" },
  { "WAIT", "PM_WAIT" },
  { "PULSE", "PM_PULSE" },
  { "I2C", "KW_I2C" },
  { "OPEN", "KW_OPEN" },
  { "CLOSE", "KW_CLOSE" },
  { "TRUNCATE", "FS_TRUNCATE" },
  { "APPEND", "FS_APPEND" },
  { "EOF", "FUNC_EOF" },
  //
  // Constants
  //
  { "TRUE", "KW_CONSTANT,CO_TRUE" },
  { "FALSE", "KW_CONSTANT,CO_FALSE" },
  { "ON", "KW_CONSTANT,CO_ON" },
  { "OFF", "KW_CONSTANT,CO_OFF" },
  { "YES", "KW_CONSTANT,CO_YES" },
  { "NO", "KW_CONSTANT,CO_NO" },
  { "HIGH", "KW_CONSTANT,CO_HIGH" },
  { "LOW", "KW_CONSTANT,CO_LOW" },

  { "ADVERT_ENABLED", "KW_CONSTANT,CO_ADVERT_ENABLED" },
  { "MIN_CONN_INTERVAL", "KW_CONSTANT,CO_MIN_CONN_INTERVAL" },
  { "MAX_CONN_INTERVAL", "KW_CONSTANT,CO_MAX_CONN_INTERVAL" },
  { "SLAVE_LATENCY", "KW_CONSTANT,CO_SLAVE_LATENCY" },
  { "TIMEOUT_MULTIPLIER", "KW_CONSTANT,CO_TIMEOUT_MULTIPLIER" },
  { "LIM_DISC_ADV_INT_MIN", "KW_CONSTANT,CO_LIM_DISC_INT_MIN" },
  { "LIM_DISC_ADV_INT_MAX", "KW_CONSTANT,CO_LIM_DISC_INT_MAX" },
  { "GEN_DISC_ADV_INT_MIN", "KW_CONSTANT,CO_GEN_DISC_INT_MIN" },
  { "GEN_DISC_ADV_INT_MAX", "KW_CONSTANT,CO_GEN_DISC_INT_MAX" },
  { "TXPOWER", "KW_CONSTANT,CO_TXPOWER" },
  { "RXGAIN", "KW_CONSTANT,CO_RXGAIN" },
  { "DEV_ADDRESS", "KW_CONSTANT,CO_DEV_ADDRESS" },
  { "GEN_DISC_ADV_MIN", "KW_CONSTANT,CO_GEN_DISC_ADV_MIN" },
  { "LIM_ADV_TIMEOUT", "KW_CONSTANT,CO_LIM_ADV_TIMEOUT" },
  { "PASSCODE", "KW_CONSTANT,CO_DEFAULT_PASSCODE" },
  { "BONDING_ENABLED", "KW_CONSTANT,CO_BONDING_ENABLED" },

  { "POWER", "KW_CONSTANT,CO_POWER" },
};

#define	NR_TABLES	13

struct keyword keyword_tables[NR_TABLES][128];


static int cmp(const void* a, const void* b)
{
  struct keyword* ka = (struct keyword*)a;
  struct keyword* kb = (struct keyword*)b;

  if (ka == kb)
  {
    return 0;
  }
  if (ka->keyword == NULL)
  {
    return 1;
  }
  if (kb->keyword == NULL)
  {
    return -1;
  }
  if (strstr(ka->keyword, kb->keyword))
  {
    return -1;
  }
  if (strstr(kb->keyword, ka->keyword))
  {
    return 1;
  }
  return strcmp(ka->keyword, kb->keyword);
}

int main()
{
  int i;
  int j;
  int t;

  for (i = 0; i < sizeof(keywords) / sizeof(struct keyword); i++)
  {
    int hash = (keywords[i].keyword[0]) % NR_TABLES;
    for (j = 0; ; j++)
    {
      if (keyword_tables[hash][j].keyword == NULL)
      {
        keyword_tables[hash][j] = keywords[i];
        break;
      }
    }
  }

  for (t = 0; t < NR_TABLES; t++)
  {
    qsort(keyword_tables[t], 128, sizeof(struct keyword), cmp);

    printf("static const unsigned char keywords_%d[] =\n{\n", t);
    for (i = 0; keyword_tables[t][i].keyword; i++)
    {
     printf("  ");
     for (j = 0; keyword_tables[t][i].keyword[j]; j++)
     {
      printf("'%c',", keyword_tables[t][i].keyword[j]);
     }
     printf("%s,\n", keyword_tables[t][i].token);
    }
    printf("  0\n};\n");
  }

  printf("static const unsigned char* keywords[] =\n{\n");
  for (t = 0; t < NR_TABLES; t++)
  {
    printf("  keywords_%d,\n", t);
  }
  printf("  0\n};\n");
  printf("#define KEYWORD_TABLE(K) keywords[(K)[0] %% %d]\n", NR_TABLES);
 
  exit(0);
}
