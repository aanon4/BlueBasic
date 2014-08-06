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
  { "LOAD", "KW_LOAD" },
  { "NEW", "KW_NEW" },
  { "RUN", "KW_RUN" },
  { "SAVE", "KW_SAVE" },
  { "NEXT", "KW_NEXT" },
  { "LET", "KW_LET" },
  { "IF", "KW_IF" },
  { "ELSE", "KW_ELSE" },
  { "ELIF", "KW_ELIF" },
  { "GOTO", "KW_GOTO" },
  { "GOSUB", "KW_GOSUB" },
  { "RETURN", "KW_RETURN" },
  { "REM", "KW_REM" },
  { "FOR", "KW_FOR" },
  { "TO", "KW_TO" },
  { "STEP", "KW_STEP" },
  { "PRINT", "KW_PRINT" },
  { "STOP", "KW_STOP" },
  { "REBOOT", "KW_REBOOT" },
  { "MEM", "KW_MEM" },
  { "DIM", "KW_DIM" },
  { "END", "KW_END" },
  { "TIMER", "KW_TIMER" },
  { "REPEAT", "KW_REPEAT" },
  { "DELAY", "KW_DELAY" },
  { "ABS(", "FUNC_ABS" },
  { "RND(", "FUNC_RND" },
  { "MILLIS(", "FUNC_MILLIS" },
  { "BATTERY(", "FUNC_BATTERY" },
  { "AUTORUN", "KW_AUTORUN" },
  { ">=", "RELOP_GE" },
  { "<>", "RELOP_NE" },
  { ">", "RELOP_GT" },
  { "==", "RELOP_EQEQ" },
  { "=", "RELOP_EQ" },
  { "<=", "RELOP_LE" },
  { "<", "RELOP_LT" },
  { "!=", "RELOP_NE_BANG" },
  { "0X", "FUNC_HEX" },
  { "+", "OP_ADD" },
  { "-", "OP_SUB" },
  { "*", "OP_MUL" },
  { "/", "OP_DIV" },
  { "%", "OP_REM" },
  { "&", "OP_AND" },
  { "|", "OP_OR" },
  { "^", "OP_XOR" },
  { "DSAVE", "KW_DSAVE" },
  { "DLOAD", "KW_DLOAD" },
  { "PINMODE", "KW_PINMODE" },
  { "ATTACHINTERRUPT", "KW_ATTACHINTERRUPT" },
  { "DETACHINTERRUPT", "KW_DETACHINTERRUPT" },
  { "PULLUP", "PM_PULLUP" },
  { "PULLDOWN", "PM_PULLDOWN" },
  { "INPUT", "PM_INPUT" },
  { "ADC", "PM_ADC" },
  { "OUTPUT", "PM_OUTPUT" },
  { "RISING", "PM_RISING" },
  { "FALLING", "PM_FALLING" },
  { "P0(", "PIN_P0" },
  { "P1(", "PIN_P1" },
  { "P2(", "PIN_P2" },
  { "ANALOGREFERENCE", "KW_ANALOGREFERENCE" },
  { "ANALOGRESOLUTION", "KW_ANALOGRESOLUTION" },
  { "INTERNAL", "KW_INTERNAL" },
  { "EXTERNAL", "KW_EXTERNAL" },
  { "ONREAD", "BLE_ONREAD" },
  { "ONWRITE", "BLE_ONWRITE" },
  { "ONCONNECT", "BLE_ONCONNECT" },
  { "TRUE", "CO_TRUE" },
  { "FALSE", "CO_FALSE" },
  { "ON", "CO_ON" },
  { "OFF", "CO_OFF" },
  { "ADVERT_ENABLED", "CO_ADVERT_ENABLED" },
  { "MIN_CONN_INTERVAL", "CO_MIN_CONN_INTERVAL" },
  { "MAX_CONN_INTERVAL", "CO_MAX_CONN_INTERVAL" },
  { "SLAVE_LATENCY", "CO_SLAVE_LATENCY" },
  { "TIMEOUT_MULTIPLIER", "CO_TIMEOUT_MULTIPLIER" },
  { "LIM_DISC_ADV_INT_MIN", "CO_LIM_DISC_INT_MIN" },
  { "LIM_DISC_ADV_INT_MAX", "CO_LIM_DISC_INT_MAX" },
  { "GEN_DISC_ADV_INT_MIN", "CO_GEN_DISC_INT_MIN" },
  { "GEN_DISC_ADV_INT_MAX", "CO_GEN_DISC_INT_MAX" },
  { "GATT", "BLE_GATT" },
  { "SERVICE", "BLE_SERVICE" },
  { "CHARACTERISTIC", "BLE_CHARACTERISTIC" },
  { "READ", "BLE_READ" },
  { "WRITENORSP", "BLE_WRITENORSP" },
  { "WRITE", "BLE_WRITE" },
  { "NOTIFY", "BLE_NOTIFY" },
  { "INDICATE", "BLE_INDICATE" },
  { "ADVERT", "BLE_ADVERT" },
  { "SCAN", "BLE_SCAN" },
  { "GENERAL", "BLE_GENERAL" },
  { "LIMITED", "BLE_LIMITED" },
  { "MORE", "BLE_MORE" },
  { "NAME", "BLE_NAME" },
  { "CUSTOM", "BLE_CUSTOM" },
  { "GAPROLE(", "BLE_FUNC_GAPROLE" },
  { "GAPROLE", "BLE_GAPROLE" },
  { "GAP(", "BLE_FUNC_GAP" },
  { "GAP", "BLE_GAP" },
  { "SPI", "KW_SPI" },
  { "TRANSFER", "KW_TRANSFER" },
  { "MSB", "KW_MSB" },
  { "LSB", "KW_LSB" },
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

main()
{
  int i;
  int j;
  int t;

  for (i = 0; i < sizeof(keywords) / sizeof(struct keyword); i++)
  {
    int hash = keywords[i].keyword[0] % NR_TABLES;
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
