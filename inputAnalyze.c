#include "defaults.h"
#include "inputAnalyze.h"

char whitespace[7] = " \t\n\v\f\r";

Statement getLine(FILE *f,char **str) {
    Statement state;
    
    fgets(*str,LINE_LEN,f);

    state = firstCheck(*str);
    return state;
}

/* return the statement of the sentence or invalid*/
Statement firstCheck(char *str) {
    char arr[STRING_PARTS][LINE_LEN];
    memset(arr,0,sizeof(arr[0][0])*STRING_PARTS*LINE_LEN);
    if(str[strlen(str)-1] != '\n') /* line too long */ 
        return Invalid;
    if(str[0] == '\n' || str[0] == '\0') /* first char is \0 */
        return Empty;
    if(str[0] == ';') /* first char is ; */
        return Comment;
    if(validToken(".",str))
        return Directive;
    return Instruction;
}

/* strip the string str from whitespaces and return the stripped string */
char *strip(char *str) {
    size_t size;
    char *end;
    /* if string empty, return it */
    size = strlen(str) - 1;
    if (!size)
        return str;

    /* remove all whitespaces from the end of the string */
    end = str + size - 1;
    while(end >= str && isspace(*end))
        --end;
    *(end + 1) = '\0';

    /* remove all whitespaces from the start of the string */
    while(str && isspace(*str))
        str++;

    return str;
}

/* split the string str by the delimeters delim
 * arr[0] will contain the string before the delim, and arr[1] will contain the string after
 * return if the delimeters exists, same as arr[1] = "" */
int split(char *str, char *delim, char arr[STRING_PARTS][LINE_LEN]) {
    char *tok;
    char strCopy[1000];
    int i;

    strcpy(strCopy, str);
    /* check if the delimeter on the first char */
    for(i=0;i<strlen(delim);i++) {
        if(str[0] == delim[i]) {
            strcpy(arr[IMPORTANT],"");
            strcpy(arr[REST],strCopy);
            return DELIM_EXIST;
        }
    }

    tok = strtok(strCopy, delim); /* look for the first token */
    if (strlen(tok) == strlen(str)) {
        strcpy(arr[IMPORTANT], str);
        strcpy(arr[REST], "");
        return DELIM_NOT_EXIST;
    }
    tok = strtok(NULL, delim); /* look for the next token */
    strcpy(arr[IMPORTANT], strCopy);
    strcpy(arr[REST], (str + strlen(strCopy) + 1)); /* we want the rest of the string, and not until the next token */
    return DELIM_EXIST;
}

/* returns the opcode of the str, -1 if not an opcode */
int findOpcode(char *str) {
    char arr[STRING_PARTS][LINE_LEN];
    char oper[LINE_LEN];
    split(str,whitespace,arr);
    strcpy(oper,arr[IMPORTANT]);
    if(!strcmp(oper,"mov"))
        return 0;
    if(!strcmp(oper,"cmp"))
        return 1;
    if(!strcmp(oper,"add") || !strcmp(oper,"sub"))
        return 2;
    if(!strcmp(oper,"lea"))
        return 4;
    if(!strcmp(oper,"clr") || !strcmp(oper,"not") || !strcmp(oper,"inc") || !strcmp(oper,"dec"))
        return 5;
    if(!strcmp(oper,"jmp") || !strcmp(oper,"bne") || !strcmp(oper,"jne"))
        return 9;
    if(!strcmp(oper,"red"))
        return 12;
    if(!strcmp(oper,"prn"))
        return 13;
    if(!strcmp(oper,"rts"))
        return 14;
    if(!strcmp(oper,"stop"))
        return 15;
    return -1;
}

/* return the register number of str, -1 if not a register */
int findReg(char *str) {
    char arr[STRING_PARTS][LINE_LEN];
    char oper[LINE_LEN];
    split(str,whitespace,arr);
    strcpy(oper,strip(arr[IMPORTANT]));

    if(!strcmp(oper,"r0"))
        return 0;
    if(!strcmp(oper,"r1"))
        return 1;
    if(!strcmp(oper,"r2"))
        return 2;
    if(!strcmp(oper,"r3"))
        return 3;
    if(!strcmp(oper,"r4"))
        return 4;
    if(!strcmp(oper,"r5"))
        return 5;
    if(!strcmp(oper,"r6"))
        return 6;
    if(!strcmp(oper,"r7"))
        return 7;
    return -1;
}

/* return the funct of str, 0 if there is no funct */
int findFunct(char *str) {
    char arr[STRING_PARTS][LINE_LEN];
    char oper[LINE_LEN];
    split(str,whitespace,arr);
    strcpy(oper,arr[IMPORTANT]);
    if(!strcmp(oper,"add") || !strcmp(oper,"clr") || !strcmp(oper,"jmp"))
        return 1;
    if(!strcmp(oper,"sub") || !strcmp(oper,"not") || !strcmp(oper,"bne"))
        return 2;
    if(!strcmp(oper,"inc") || !strcmp(oper,"jsr"))
        return 3;
    if(!strcmp(oper,"dec"))
        return 4;
    return 0;
}

/* return NULL if the token is not valid, or the string if this is a real token */
char *validToken(char *tok, char *str) {
    char arr[STRING_PARTS][LINE_LEN];
    char *tempStr = (char*) malloc(LINE_LEN);
    int del;
    size_t len1,len2;

    del = split(str,tok,arr);
    if(del == DELIM_NOT_EXIST)
        return NULL;
    len1 = strlen(arr[IMPORTANT]);
    strcpy(tempStr,arr[IMPORTANT]);
    split(str,"\"",arr);
    len2 = strlen(arr[IMPORTANT]);
    if(len1 > len2)
        return NULL;
    return tempStr;
}

/* return the type of the line */
Type findEntryOrExternal(char *str) {
    EntryOrExternal flag = None;
    if(!strncmp(str,".extern ", 8) || !strncmp(str,".extern\t", 8))
        flag = External;
    else if(!strcmp(str,".entry ", 7) || !strcmp(str,".entry\t", 7))
        flag = Entry;
    return flag;
}

DataOrString findDataOrString(char *str) {
    DataOrString flag = None;
    if(!strncmp(str,".data ",6) || !strncmp(str,".data\t",6))
        flag = DataVar;
    else if(!strncmp(str,".string ",8) || !strncmp(str,".string\t",8))
        flag = StringVar;
    return None;
}

/* find the first occurence of ch in the str and return the len, if not found return -1 */
int findFromEnd(char *str, char ch) {
    int i;
    for(i=strlen(str)-1;i>=0;i++) {
        if(str[i] == ch)
            return strlen(str) - i;
    }
    return -1;
}