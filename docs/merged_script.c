
/* --- START OF ./compiler.c --- */

#include "data.h"

int main(int argc, char *argv[])
{
    /*
    we are passing the command line arguments that are the files that passed to the assembler
    to the handleSourceFiles function.
     */
    handleSourceFiles(argc, argv);
    return 0;
}
int handleSourceFiles(int argc, char *argv[])
{
    int filesCount = argc - 1;
    int i = 1;
    if (filesCount < 1)
    {
        fprintf(stderr, "\n\nYou did not passed any source files to the assembler!\n\n");
        exit(1);
    }
    while (--argc)
    {
        handleSingleFile(argv[i]);
        i++;
    }
    closeOpenLogFiles();

    return 0;
}
void handleSingleFile(char *arg)
{
    FILE *src = NULL, *target = NULL;
    void (*setPath)(char *) = &setFileNamePath;
    State (*globalState)() = &getGlobalState;
    void (*setState)(State) = &setGlobalState;

    char *fileName = (char *)calloc(strlen(arg) + 4, sizeof(char *));
    extern void resetMemoryCounters();
    extern void initTables();
    extern void exportFilesMainHandler();
    extern void closeOpenLogFiles();
    extern void allocMemoryImg();
    extern void calcFinalAddrsCountersValues();
    extern void freeHashTable(ItemType type);

    strncpy(fileName, arg, strlen(arg)); /* sets the fileName to be arg */
    strcat(fileName, ".as");             /* turns the files into .as files */
    (*setPath)(fileName);
    /* if the file couldn't be opened it alerts it to the user */
    if ((src = fopen(fileName, "r")) == NULL)
    {
        fprintf(stderr, "\n######################################################################\n");
        fprintf(stderr, " FAILURE! source code file %s could not be opened\n", fileName);
        fprintf(stderr, "######################################################################\n\n");
        free(fileName);
        return;
    }

    fileName[strlen(fileName) - 1] = 'm'; /* change the files to be .am files */
    (*setPath)(fileName);
    /* if the expanded soure file couldn't be created it alerts it to the user */
    if ((target = fopen(fileName, "w+")) == NULL)
    {

        fprintf(stderr, "\n######################################################################\n");
        fprintf(stderr, " FAILURE! expanded source code file %s could not be created\n", fileName);
        fprintf(stderr, "######################################################################\n\n");
        fclose(src);
        free(fileName);
        return;
    }

    else
    {
        initTables();

        /* if there are no errors it starts the pre proccesing- parse all the macros,
         saves them in the macro table and prints it */
        (*setState)(parsingMacros);
        resetMemoryCounters();
        parseSourceFile(src, target);
        printMacroTable();
        freeHashTable(Macro);

        /* moves on to the first run, looks for errors in the code, counts how much space in the
         memory the program needs and starts to parse the assembly code.
         */
        if ((*globalState)() == firstRun)
        {

            rewind(target);
            parseAssemblyCode(target);
            /* if the first run ended with no errors, it moves on to the second run,
            builds the memory image, prints the symbols table */
            if ((*globalState)() == secondRun)
            {
                calcFinalAddrsCountersValues();
                updateFinalSymbolTableValues();
                allocMemoryImg();
                printSymbolTable();
                rewind(target);
                parseAssemblyCode(target);
                /* if the second run ended with no errors, it moves to export file
                and creates the additional files (.ob, .ent and .ext files) */
                if ((*globalState)() == exportFiles)
                {
                    fileName[strlen(fileName) - 3] = '\0';
                    (*setPath)(fileName);
                    exportFilesMainHandler();
                }
                else
                    printf("\nSecond Run Finished With Errors, files will not be exported!\n");
            }
            else
                printf("\nFirst Run Finished With Errors, will no enter second run and files will not be exported!\n");
            freeHashTable(Symbol);
        }
        else
            printf("\nexpanding macros for %s source file\nfailed due to some code errors\nmoving on to the next file if exist\n\n", fileName);

        /* frees and closes all the files that have been in use */
        free(fileName);
        fclose(src);
        fclose(target);
    }
}
/* --- END OF ./compiler.c --- */


/* --- START OF ./data.h --- */

#include "./headers/lib/lib.h"
#include "./headers/variables/variables.h"
#include "./headers/functions/functions.h"
/* --- END OF ./data.h --- */


/* --- START OF ./errors.c --- */

#include "data.h"

static int (*line)() = &getCurrentLineNumber;
static char *(*filePath)() = &getFileNamePath;
static FILE *warningsFile = NULL, *errorsFile = NULL;
static Bool isWarningFileExist = False;
static Bool isErrorFileExist = False;

void fileCreationFailure(char *fileName)
{
    extern FILE *errorsFile;
    extern Bool isErrorFileExist;

    if (!isErrorFileExist)
    {
        if ((errorsFile = fopen("errors.log", "w+")) == NULL)
        {
            fprintf(stderr, "\n######################################################################\n");
            fprintf(stderr, " FAILURE! failed to create %s error log file\n", fileName);
            fprintf(stderr, "######################################################################\n\n");
        }
        else
            isErrorFileExist = True;
    }

    fprintf(stderr, "\n######################################################################\n");
    fprintf(stderr, " FAILURE! failed to create %s file\n", fileName);
    fprintf(stderr, "######################################################################\n\n");

    fprintf(errorsFile, "\n######################################################################\n");
    fprintf(errorsFile, " FAILURE! failed to create %s file\n", fileName);
    fprintf(errorsFile, "######################################################################\n\n");
}

void fileOpeningFailure(char *fileName)
{
    extern FILE *errorsFile;
    extern Bool isErrorFileExist;

    if (!isErrorFileExist)
    {
        if ((errorsFile = fopen("errors.log", "w+")) == NULL)
        {
            fprintf(stderr, "\n######################################################################\n");
            fprintf(stderr, " FAILURE! failed to create %s error log file\n", fileName);
            fprintf(stderr, "######################################################################\n\n");
        }
        else
            isErrorFileExist = True;
    }

    fprintf(stderr, "\n######################################################################\n");
    fprintf(stderr, " FAILURE! failed to open %s file\n", fileName);
    fprintf(stderr, "######################################################################\n\n");

    fprintf(errorsFile, "\n######################################################################\n");
    fprintf(errorsFile, " FAILURE! failed to open %s file\n", fileName);
    fprintf(errorsFile, "######################################################################\n\n");
}

void yieldWarningIntoFile(Warning err)
{
    extern Bool isWarningFileExist;
    extern FILE *warningsFile;
    if (!isWarningFileExist)
    {

        if ((warningsFile = fopen("warnings.log", "w+")) == NULL)
            printf("Failed to open warning log file\n");
        else
            isWarningFileExist = True;
    }

    fprintf(warningsFile, "\n######################################################################\n");
    fprintf(warningsFile, "Warning!! in %s on line number %d\n", (*filePath)(), (*line)());
    switch (err)
    {

    case emptyDataDeclaretion:
        fprintf(warningsFile, "empty Data Declaretion");
        break;

    case emptyExternalDeclaretion:
        fprintf(warningsFile, "empty external Declatretion");
        break;

    case emptyEntryDeclaretion:
        fprintf(warningsFile, "empty entry Declatretion");
        break;
    case emptyDeclaretionOfEntryOrExternalVariables:
        fprintf(warningsFile, "empty Declaretion Of Entry Or External Variables");
        break;
    case instructionHasNoArguments:
        fprintf(warningsFile, "instruction Has No Arguments");
        break;

    default:
        break;
    }

    fprintf(warningsFile, "\n");
    fprintf(warningsFile, "######################################################################\n\n");
}

void yieldErrorIntoFile(Error err)
{
    extern FILE *errorsFile;
    extern Bool isErrorFileExist;
    if (!isErrorFileExist)
    {
        if ((errorsFile = fopen("errors.log", "w+")) == NULL)
            printf("Failed to open erro log file\n");
        else
            isErrorFileExist = True;
    }
    fprintf(errorsFile, "\n######################################################################\n");
    fprintf(errorsFile, "Error!! occured in %s on line number %d\n", (*filePath)(), (*line)());

    switch (err)
    {
    case macroDeclaretionWithoutDefiningMacroName:
        fprintf(errorsFile, "Macro declaration without defining macro name!");
        break;

    case registeryIndexOperandTypeIfOutOfAllowedRegisteriesRange:
        fprintf(errorsFile, "illegal use of registery index operand addressing method\nRegistery Index addr method is allows to use only registeries\n that are between the r10 - r15!");
        break;
    case illegalInputPassedAsOperandSrcOperand:
        fprintf(errorsFile, "illegal operands input passed to source operand ");
        break;

    case illegalInputPassedAsOperandDesOperand:
        fprintf(errorsFile, "illegal operands input passed to destination operand ");
        break;
    case undefinedLabelDeclaretion:
        fprintf(errorsFile, "undefined Label Declaretion");
        break;
    case emptyLabelDecleration:
        fprintf(errorsFile, "empty Label Declaretion");
        break;
    case emptyStringDeclatretion:
        fprintf(errorsFile, "empty String Declatretion");
        break;
    case missingSpaceBetweenLabelDeclaretionAndInstruction:
        fprintf(errorsFile, "Missing space between label declaretion and instruction name");
        break;
    case extraOperandsPassed:
        fprintf(errorsFile, "Extra operands passed as paramters");
        break;
    case wrongCommasSyntaxIllegalApearenceOfCommasInLine:
        fprintf(errorsFile, "Illegal apearence of commas in line");
        break;
    case wrongCommasSyntaxExtra:
        fprintf(errorsFile, "Extra commas between arguments");
        break;

    case wrongCommasSyntaxMissing:
        fprintf(errorsFile, "Missing Commas between arguments");
        break;
    case illegalLabelNameLength:
        fprintf(errorsFile, "illegal Label Name length is greater than the maximum allowed which is %d characters", MAX_LABEL_LEN);
        break;
    case illegalLabelNameUseOfSavedKeywordUsingOperationName:
        fprintf(errorsFile, "illegal Label Name Use Of Saved Keyword.\nUsing Operation Name is not allowed");
        break;

    case illegalLabelNameUseOfSavedKeywordUsingRegisteryName:
        fprintf(errorsFile, "illegal Label Name Use Of Saved Keyword.\nUsing Registery Name is not allowed");
        break;

    case illegalLabelNameUseOfSavedKeywords:
        fprintf(errorsFile, "illegal Label Name Use Of Saved Keyword.\n");
        break;
    case desOperandTypeIsNotAllowed:
        fprintf(errorsFile, "type of destination operand passed to operation is not accepted!");
        break;

    case srcOperandTypeIsNotAllowed:
        fprintf(errorsFile, "type of source operand passed to operation is not accepted!");
        break;
    case illegalOverrideOfLocalSymbolWithExternalSymbol:
        fprintf(errorsFile, "symbol already declared and defined locally,\nso it could not be re-declared as external variable.");
        break;
    case illegalApearenceOfCharactersInTheEndOfTheLine:
        fprintf(errorsFile, "illegal apearence of extra characters in the end of the line");
        break;
    case afterPlusOrMinusSignThereMustBeANumber:
        fprintf(errorsFile, "after Plus Or Minus Sign There Must Be A Number without any spaces between");
        break;

    case useOfNestedMacrosIsIllegal:
    {
        fprintf(errorsFile, "useOfNestedMacrosIsIllegal");
        break;
    }

    case macroClosingWithoutAnyOpenedMacro:
        fprintf(errorsFile, "macroClosingWithoutAnyOpenedMacro");
        break;
    case missinSpaceAfterInstruction:
    {
        fprintf(errorsFile, "missin Space between instruction and arguments");
        break;
    }
    case illegalApearenceOfCommaBeforeFirstParameter:
    {
        fprintf(errorsFile, "Illegal appearence of a comma before the first parameter");
        break;
    }
    case illegalApearenceOfCommaAfterLastParameter:
    {
        fprintf(errorsFile, "Illegal appearence of a comma after the last parameter");
        break;
    }

    case wrongInstructionSyntaxIllegalCommaPosition:
    {
        fprintf(errorsFile, "comma appearence on line is illegal!");
        break;
    }
    case labelNotExist:
    {
        fprintf(errorsFile, "undefined label name, label name was not found in symbol table, assembler compilation faild, not files were created");
        break;
    }

    case operandTypeDoNotMatch:
    {
        fprintf(errorsFile, "Operand type does not fit to current operation");
        break;
    }
    case entryDeclaredButNotDefined:
        fprintf(errorsFile, "operand is registered as .entry in table but is not defined in the file");
        break;
    case requiredSourceOperandIsMissin:
    {
        fprintf(errorsFile, "required source operand is missing");
        break;
    }

    case requiredDestinationOperandIsMissin:
    {
        fprintf(errorsFile, "required destination operand is missing");
        break;
    }
    case illegalMacroNameUseOfSavedKeywords:
        fprintf(errorsFile, "illegal Macro Name Use Of Saved Keywords");
        break;
    case wrongRegisteryReferenceUndefinedReg:
        fprintf(errorsFile, "undefined registery, registeries names are r0 - r15");
        break;

    case fileCouldNotBeOpened:
        fprintf(errorsFile, "file could not be  opened");
        break;

    case illegalOverrideOfExternalSymbol:
        fprintf(errorsFile, "Overriding of external symbol exisiting in table is not allowed!");
        break;
    case memoryAllocationFailure:
        fprintf(errorsFile, "memory allocation failed");
        break;
    case undefinedOperation:
        fprintf(errorsFile, "undefined operation name");
        break;
    case undefinedInstruction:
        fprintf(errorsFile, "undefined instruction name");
        break;
    case tooMuchArgumentsPassed:
        fprintf(errorsFile, "too Much Arguments Passed");
        break;
    case notEnoughArgumentsPassed:
        fprintf(errorsFile, "too little arguments passed");
        break;
    case symbolDoesNotExist:
        fprintf(errorsFile, "symbol does not exist");
        break;
    case macroDoesNotExist:
        fprintf(errorsFile, "macro does not exist");
        break;
    case wrongArgumentTypePassedAsParam:
        fprintf(errorsFile, "wrong Argument Type Passed As Param");
        break;
    case illegalMacroNameUseOfCharacters:
        fprintf(errorsFile, "illegal Macro Name Use Of Characters");
        break;
    case illegalLabelNameUseOfCharacters:
        fprintf(errorsFile, "illegal Label Name Use Of Characters");
        break;

    case illegalLabelDeclaration:
        fprintf(errorsFile, "illegal Label Declaration");
        break;
    case illegalMacroNameLength:
        fprintf(errorsFile, "illegal Macro Name Length");
        break;
    case illegalSymbolNameAlreadyInUse:
        fprintf(errorsFile, "Label Name Already In Use");
        break;
    case illegalMacroNameAlreadyInUse:
        fprintf(errorsFile, "Macro Name Already In Use");
        break;
    case wrongArgumentTypeNotAnInteger:
        fprintf(errorsFile, "Number must be an integer!");
        break;
    case expectedNumber:
        fprintf(errorsFile, "expected number but got letter instead, argument must be an integer number");
        break;

    case symbolCannotBeBothCurrentTypeAndRequestedType:
        fprintf(errorsFile, "symbol Cannot Be Both Current Type And Requested Type");
        break;
    case illegalLabelUseExpectedOperationOrInstruction:
        fprintf(errorsFile, "illegal Label Use Expected Operation Or Instruction");
        break;
    case wrongInstructionSyntaxMissinCommas:
    {
        fprintf(errorsFile, "Missing Comma between argumentes");
        break;
    }

    case wrongOperationSyntaxMissingCommas:
        fprintf(errorsFile, "Missing Comma between operands");
        break;
    case wrongOperationSyntaxExtraCommas:
        fprintf(errorsFile, "Extra Comma between operands");
        break;

    case wrongInstructionSyntaxExtraCommas:
    {
        fprintf(errorsFile, "extra comma between arguments");
        break;
    }

    case expectedSingleCommaCharacter:
        fprintf(errorsFile, "Missing Comma");
        break;
    case illegalApearenceOfCharactersOnLine:
        fprintf(errorsFile, "Illegal use of characters");
        break;
    case undefinedTokenNotOperationOrInstructionOrLabel:
        fprintf(errorsFile, "illegal token in beginning of current line,  not operation nor instruction or label decleration");
        break;
    case illegalApearenceOfExtraCharactersOnLine:
        fprintf(errorsFile, "Illegal appearence of extra characters after the end or before begning of the line");
        break;

    case illegalApearenceOfCharacterInTheBegningOfTheLine:
        fprintf(errorsFile, "Illegal appearence of characters at beginning of the line");
        break;

    case expectedQuotes:
        fprintf(errorsFile, "expected opening quotes before string");
        break;

    case closingQuotesForStringIsMissing:
        fprintf(errorsFile, "missin closing quotes of string variable");
        break;

    case maxLineLengthExceeded:
        fprintf(errorsFile, "line character length is illegal");

        break;
    default:
        break;
    }
    fprintf(errorsFile, "\n");
    fprintf(errorsFile, "######################################################################\n");
}
Bool yieldWarning(Warning err)
{
    yieldWarningIntoFile(err);
    fprintf(stderr, "\n######################################################################\n");
    fprintf(stderr, "Warning!! in %s on line number %d\n", (*filePath)(), (*line)());
    switch (err)
    {

    case emptyDataDeclaretion:
        fprintf(stderr, "empty Data Declaretion");
        break;

    case emptyExternalDeclaretion:
        fprintf(stderr, "empty external Declatretion");
        break;

    case emptyEntryDeclaretion:
        fprintf(stderr, "empty entry Declatretion");
        break;
    case emptyDeclaretionOfEntryOrExternalVariables:
        fprintf(stderr, "empty Declaretion Of Entry Or External Variables");
        break;
    case instructionHasNoArguments:
        fprintf(stderr, "instruction Has No Arguments");
        break;

    default:
        break;
    }

    fprintf(stderr, "\n");
    fprintf(stderr, "######################################################################\n\n");
    return True;
}

Bool yieldError(Error err)
{
    yieldErrorIntoFile(err);
    fprintf(stderr, "\n######################################################################\n");
    fprintf(stderr, "Error!! occured in %s on line number %d\n", (*filePath)(), (*line)());

    switch (err)
    {

    case macroDeclaretionWithoutDefiningMacroName:
        fprintf(stderr, "Macro declaration without defining macro name!");
        break;
    case registeryIndexOperandTypeIfOutOfAllowedRegisteriesRange:
        fprintf(stderr, "illegal use of registery index operand addressing method\nRegistery Index addr method is allows to use only registeries\n that are between the r10 - r15!");
        break;
    case illegalInputPassedAsOperandSrcOperand:
        fprintf(stderr, "illegal operands input passed to source operand ");
        break;

    case illegalInputPassedAsOperandDesOperand:
        fprintf(stderr, "illegal operands input passed to destination operand ");
        break;
    case desOperandTypeIsNotAllowed:
        fprintf(stderr, "type of destination operand passed to operation is not accepted!");
        break;

    case srcOperandTypeIsNotAllowed:
        fprintf(stderr, "type of source operand passed to operation is not accepted!");
        break;
    case undefinedLabelDeclaretion:
        fprintf(stderr, "undefined Label Declaretion");
        break;
    case emptyLabelDecleration:
        fprintf(stderr, "empty Label Declaretion");
        break;
    case emptyStringDeclatretion:
        fprintf(stderr, "empty String Declatretion");
        break;
    case missingSpaceBetweenLabelDeclaretionAndInstruction:
        fprintf(stderr, "Missing space between label declaretion and instruction name");
        break;
    case illegalLabelNameLength:
        fprintf(stderr, "illegal Label Name length is greater than the maximum allowed which is %d characters", MAX_LABEL_LEN);
        break;
    case extraOperandsPassed:
        fprintf(stderr, "Extra operands passed as paramters");
        break;

    case wrongCommasSyntaxIllegalApearenceOfCommasInLine:
        fprintf(stderr, "Illegal apearence of commas in line");
        break;
    case wrongCommasSyntaxExtra:
        fprintf(stderr, "Extra commas between arguments");
        break;

    case wrongCommasSyntaxMissing:
        fprintf(stderr, "Missing Commas between arguments");
        break;

    case illegalLabelNameUseOfSavedKeywordUsingOperationName:
        fprintf(stderr, "illegal Label Name Use Of Saved Keyword.\nUsing Operation Name is not allowed");
        break;

    case illegalLabelNameUseOfSavedKeywordUsingRegisteryName:
        fprintf(stderr, "illegal Label Name Use Of Saved Keyword.\nUsing Registery Name is not allowed");
        break;

    case illegalLabelNameUseOfSavedKeywords:
        fprintf(stderr, "illegal Label Name Use Of Saved Keyword.\n");
        break;

    case illegalOverrideOfLocalSymbolWithExternalSymbol:
        fprintf(stderr, "symbol already declared and defined locally,\nso it could not be re-declared as external variable.");
        break;
    case illegalApearenceOfCharactersInTheEndOfTheLine:
        fprintf(stderr, "illegal apearence of extra characters in the end of the line");
        break;
    case afterPlusOrMinusSignThereMustBeANumber:
        fprintf(stderr, "after Plus Or Minus Sign There Must Be A Number without any spaces between");
        break;

    case useOfNestedMacrosIsIllegal:
    {
        fprintf(stderr, "useOfNestedMacrosIsIllegal");
        break;
    }

    case macroClosingWithoutAnyOpenedMacro:
        fprintf(stderr, "macroClosingWithoutAnyOpenedMacro");
        break;
    case missinSpaceAfterInstruction:
    {
        fprintf(stderr, "missin Space between instruction and arguments");
        break;
    }
    case illegalApearenceOfCommaBeforeFirstParameter:
    {
        fprintf(stderr, "Illegal appearence of a comma before the first parameter");
        break;
    }
    case illegalApearenceOfCommaAfterLastParameter:
    {
        fprintf(stderr, "Illegal appearence of a comma after the last parameter");
        break;
    }

    case wrongInstructionSyntaxIllegalCommaPosition:
    {
        fprintf(stderr, "comma appearence on line is illegal!");
        break;
    }
    case labelNotExist:
    {
        fprintf(stderr, "undefined label name, label name was not found in symbol table, assembler compilation faild, not files were created");
        break;
    }

    case operandTypeDoNotMatch:
    {
        fprintf(stderr, "Operand type does not fit to current operation");
        break;
    }
    case entryDeclaredButNotDefined:
        fprintf(stderr, "operand is registered as .entry in table but is not defined in the file");
        break;
    case requiredSourceOperandIsMissin:
    {
        fprintf(stderr, "required source operand is missing");
        break;
    }

    case requiredDestinationOperandIsMissin:
    {
        fprintf(stderr, "required destination operand is missing");
        break;
    }
    case illegalMacroNameUseOfSavedKeywords:
        fprintf(stderr, "illegal Macro Name Use Of Saved Keywords");
        break;
    case wrongRegisteryReferenceUndefinedReg:
        fprintf(stderr, "undefined registery, registeries names are r0 - r15");
        break;

    case fileCouldNotBeOpened:
        fprintf(stderr, "file could not be  opened");
        break;

    case illegalOverrideOfExternalSymbol:
        fprintf(stderr, "Overriding of external symbol exisiting in table is not allowed!");
        break;
    case memoryAllocationFailure:
        fprintf(stderr, "memory allocation failed");
        break;
    case undefinedOperation:
        fprintf(stderr, "undefined operation name");
        break;
    case undefinedInstruction:
        fprintf(stderr, "undefined instruction name");
        break;
    case tooMuchArgumentsPassed:
        fprintf(stderr, "too Much Arguments Passed");
        break;
    case notEnoughArgumentsPassed:
        fprintf(stderr, "too little arguments passed");
        break;
    case symbolDoesNotExist:
        fprintf(stderr, "symbol does not exist");
        break;
    case macroDoesNotExist:
        fprintf(stderr, "macro does not exist");
        break;
    case wrongArgumentTypePassedAsParam:
        fprintf(stderr, "wrong Argument Type Passed As Param");
        break;
    case illegalMacroNameUseOfCharacters:
        fprintf(stderr, "illegal Macro Name Use Of Characters");
        break;
    case illegalLabelNameUseOfCharacters:
        fprintf(stderr, "illegal Label Name Use Of Characters");
        break;
    case illegalLabelDeclaration:
        fprintf(stderr, "illegal Label Declaration");
        break;
    case illegalMacroNameLength:
        fprintf(stderr, "illegal Macro Name Length");
        break;
    case illegalSymbolNameAlreadyInUse:
        fprintf(stderr, "Label Name Already In Use");
        break;
    case illegalMacroNameAlreadyInUse:
        fprintf(stderr, "Macro Name Already In Use");
        break;
    case wrongArgumentTypeNotAnInteger:
        fprintf(stderr, "Number must be an integer!");
        break;
    case expectedNumber:
        fprintf(stderr, "expected number but got letter instead, argument must be an integer number");
        break;

    case symbolCannotBeBothCurrentTypeAndRequestedType:
        fprintf(stderr, "symbol Cannot Be Both Current Type And Requested Type");
        break;
    case illegalLabelUseExpectedOperationOrInstruction:
        fprintf(stderr, "illegal Label Use Expected Operation Or Instruction");
        break;
    case wrongInstructionSyntaxMissinCommas:
    {
        fprintf(stderr, "Missing Comma between argumentes");
        break;
    }

    case wrongOperationSyntaxMissingCommas:
        fprintf(stderr, "Missing Comma between operands");
        break;
    case wrongOperationSyntaxExtraCommas:
        fprintf(stderr, "Extra Comma between operands");
        break;

    case wrongInstructionSyntaxExtraCommas:
    {
        fprintf(stderr, "Extra comma between arguments");
        break;
    }

    case expectedSingleCommaCharacter:
        fprintf(stderr, "Missing Comma");
        break;
    case illegalApearenceOfCharactersOnLine:
        fprintf(stderr, "Illegal use of characters");
        break;
    case undefinedTokenNotOperationOrInstructionOrLabel:
        fprintf(stderr, "illegal token in beginning of current line,  not operation nor instruction or label decleration");
        break;
    case illegalApearenceOfExtraCharactersOnLine:
        fprintf(stderr, "Illegal appearence of extra characters after the end or before begning of the line");
        break;

    case illegalApearenceOfCharacterInTheBegningOfTheLine:
        fprintf(stderr, "Illegal appearence of characters at beginning of the line");
        break;

    case expectedQuotes:
        fprintf(stderr, "expected opening quotes before string");
        break;

    case closingQuotesForStringIsMissing:
        fprintf(stderr, "missin closing quotes of string variable");
        break;

    case maxLineLengthExceeded:
        fprintf(stderr, "line character length is illegal");

        break;
    default:
        break;
    }
    fprintf(stderr, "\n");
    fprintf(stderr, "######################################################################\n");

    return False;
}

void closeOpenLogFiles()
{
    extern FILE *warningsFile, *errorsFile;
    extern Bool isWarningFileExist, isErrorFileExist;
    if (isWarningFileExist && warningsFile != NULL)
    {
        fclose(warningsFile);
        isWarningFileExist = False;
    }
    if (isErrorFileExist && errorsFile != NULL)
    {
        fclose(errorsFile);
        isErrorFileExist = False;
    }
}
/* --- END OF ./errors.c --- */


/* --- START OF ./exportFiles.c --- */

#include "data.h"

extern void fileCreationFailure(char *fileName);
static char *(*baseFileName)() = &getFileNamePath;

/*  void exportFilesMainHandler()
This function calls other functions that will create the .ob file,
.ent file and .ext file if there are any entry labels, and .ext file
if there are any external labels.*/

void exportFilesMainHandler()
{
    printf("Finished Successfully, about to export files!\n");
    generateObFile();
    if (areEntriesExist())
        createEntriesFile();
    if (areExternalsExist())
        createExternalsFile();
}

/* void generateObFile()
This function generates the .ob files by writing the memory image,
if the file can't be generate it yields error  */
void generateObFile()
{
    FILE *ob;
    char *fileName = (*baseFileName)();
    strcat(fileName, ".ob");
    ob = fopen(fileName, "w+");

    if (ob != NULL)
    {
        writeMemoryImageToObFile(ob);
        fclose(ob);
        free(fileName);
    }
    else
        fileCreationFailure(fileName);
}

/* void createEntriesFile()
This function generates the .ent files (as long as there are entry labels) by writing
it into the file, if the file can't be generate it yields error */
void createEntriesFile()
{
    FILE *ent;
    char *fileName = (*baseFileName)();
    strcat(fileName, ".ent");
    ent = fopen(fileName, "w+");

    if (ent != NULL)
    {
        writeEntriesToFile(ent);
        fclose(ent);
        free(fileName);
    }
    else
        fileCreationFailure(fileName);
}

/* createExternalsFile()
This function generates the .ext files (as long as there are external labels)
by writing it into the file, if the file can't be generate it yields error */
void createExternalsFile()
{
    FILE *ext;
    char *fileName = (*baseFileName)();
    strcat(fileName, ".ext");
    ext = fopen(fileName, "w+");
    if (ext != NULL)
    {
        writeExternalsToFile(ext);
        fclose(ext);
        free(fileName);
    }
    else
        fileCreationFailure(fileName);
}
/* --- END OF ./exportFiles.c --- */


/* --- START OF ./firstRun.c --- */

#include "data.h"

Bool handleOperation(char *operationName, char *args)
{
    const Operation *p = getOperationByName(operationName);
    AddrMethodsOptions active[2] = {{0, 0, 0, 0}, {0, 0, 0, 0}};
    char *first = 0;
    char *second = 0;
    char *extra = 0;
    Bool areOperandsLegal;

    if (*args)
        areOperandsLegal = verifyCommaSyntax(args);

    first = strtok(args, ", \t\n\f\r");
    if (first)
    {
        second = strtok(NULL, ", \t\n\f\r");
        if (second)
        {
            extra = strtok(NULL, ", \t\n\f\r");
            if (extra)
                areOperandsLegal = yieldError(extraOperandsPassed);
        }
        else
            second = 0;
    }
    areOperandsLegal = parseOperands(first, second, p, active) && areOperandsLegal;

    if (areOperandsLegal)
    {
        int size = 2;
        if (active[0].immediate || active[1].immediate)
            size++;
        if ((active[0].direct || active[0].index) || (active[1].direct || active[1].index))
            size += 2;
        if (!p->funct && (!active[0].direct && !active[0].immediate && !active[0].index && !active[0].reg) && (!active[1].direct && !active[1].immediate && !active[1].index && !active[1].reg))
            size = 1;

        active[0].direct = active[0].immediate = active[0].index = active[0].reg = 0;
        active[1].direct = active[1].immediate = active[1].index = active[1].reg = 0;
        increaseInstructionCounter(size);
    }

    return areOperandsLegal;
}
Bool parseOperands(char *src, char *des, const Operation *op, AddrMethodsOptions active[2])
{

    int expectedOperandsCount = 0;
    int operandsPassedCount = 0;
    Bool isValid = True;
    if (src)
        operandsPassedCount++;
    if (des)
        operandsPassedCount++;
    if (op->src.direct || op->src.immediate || op->src.index || op->src.reg)
        expectedOperandsCount++;
    if (op->des.direct || op->des.immediate || op->des.index || op->des.reg)
        expectedOperandsCount++;

    if (expectedOperandsCount == 1 && operandsPassedCount == 1)
    {
        des = src;
        src = 0;
    }

    if ((expectedOperandsCount == operandsPassedCount) && expectedOperandsCount == 0)
        return True;

    if (operandsPassedCount > expectedOperandsCount)
        isValid = yieldError(extraOperandsPassed);

    if ((op->src.direct || op->src.immediate || op->src.reg || op->src.index) && (op->des.direct || op->des.immediate || op->des.reg || op->des.index))
    {

        if (!src)
            isValid = yieldError(requiredSourceOperandIsMissin);

        else
            isValid = validateOperandMatch(op->src, active, src, 0) && isValid;

        if (!des)
            isValid = yieldError(requiredDestinationOperandIsMissin);
        else
            isValid = validateOperandMatch(op->des, active, des, 1) && isValid;
    }
    else if (op->src.direct || op->src.immediate || op->src.reg || op->src.index)
    {
        if (!src)
            return yieldError(requiredSourceOperandIsMissin);
        else
            return validateOperandMatch(op->src, active, src, 0) && isValid;
    }
    else if (op->des.direct || op->des.immediate || op->des.reg || op->des.index)
    {
        if (!des)
            return yieldError(requiredDestinationOperandIsMissin);
        else
            return validateOperandMatch(op->des, active, des, 1) && isValid;
    }

    return isValid;
}
Bool validateOperandMatch(AddrMethodsOptions allowedAddrs, AddrMethodsOptions active[2], char *operand, int type)
{
    Bool isAny = isValidImmediateParamter(operand) || isValidIndexParameter(operand) || isRegistery(operand) || verifyLabelNaming(operand) || isIndexParameter(operand);
    Bool isImmediate = isValidImmediateParamter(operand);
    Bool isDirectIndex = !isImmediate && isValidIndexParameter(operand);
    Bool isReg = !isDirectIndex && !isImmediate && isRegistery(operand);
    Bool isDirect = !isReg && !isDirectIndex && !isImmediate && verifyLabelNaming(operand);

    if (isIndexParameter(operand) && !isDirectIndex)
        return yieldError(registeryIndexOperandTypeIfOutOfAllowedRegisteriesRange);

    if (!isAny)
        return type == 1 ? yieldError(illegalInputPassedAsOperandDesOperand) : yieldError(illegalInputPassedAsOperandSrcOperand);

    else if (!allowedAddrs.reg && isReg)
        return type == 1 ? yieldError(desOperandTypeIsNotAllowed) : yieldError(srcOperandTypeIsNotAllowed);
    else if (!allowedAddrs.immediate && isImmediate)
        return type == 1 ? yieldError(desOperandTypeIsNotAllowed) : yieldError(srcOperandTypeIsNotAllowed);
    else if (!allowedAddrs.direct && isDirect)
        return type == 1 ? yieldError(desOperandTypeIsNotAllowed) : yieldError(srcOperandTypeIsNotAllowed);
    else if (!allowedAddrs.index && isDirectIndex)
        return type == 1 ? yieldError(desOperandTypeIsNotAllowed) : yieldError(srcOperandTypeIsNotAllowed);

    active[type].direct = isDirect;
    active[type].reg = isReg;
    active[type].immediate = isImmediate;
    active[type].index = isDirectIndex;

    return True;
}

Bool handleInstruction(int type, char *firstToken, char *nextTokens, char *line)
{

    if (isInstruction(firstToken))
    {

        if (type == _TYPE_DATA)
        {
            return countAndVerifyDataArguments(line) ? True : False;
        }
        else if (type == _TYPE_STRING)
            return countAndVerifyStringArguments(line) ? True : False;

        if (type == _TYPE_ENTRY || type == _TYPE_EXTERNAL)
        {
            if (nextTokens)
            {
                char *labelName = cloneString(nextTokens);
                nextTokens = strtok(NULL, " \t\n\f\r");
                if (nextTokens)
                {
                    yieldError(illegalApearenceOfCharactersInTheEndOfTheLine);
                    return False;
                }
                else
                {
                    if (type == _TYPE_ENTRY)
                        return addSymbol(labelName, 0, 0, 0, 1, 0) ? True : False;
                    if (type == _TYPE_EXTERNAL)
                        return addSymbol(labelName, 0, 0, 0, 0, 1) ? True : False;
                }

                free(labelName);
            }
            else
            {
                yieldError(emptyDeclaretionOfEntryOrExternalVariables);
                return False;
            }
        }
    }
    else if (isLabelDeclaration(firstToken))
    {
        int dataCounter = getDC();
        Bool isLabelNameAvailable;
        firstToken[strlen(firstToken) - 1] = '\0';
        isLabelNameAvailable = !isLabelNameAlreadyTaken(firstToken, Symbol);
        if (!isLabelNameAvailable)
            yieldError(illegalSymbolNameAlreadyInUse);

        if (((type == _TYPE_DATA && countAndVerifyDataArguments(line)) || (type == _TYPE_STRING && countAndVerifyStringArguments(line))) && isLabelNameAvailable)
        {

            return addSymbol(firstToken, dataCounter, 0, 1, 0, 0) ? True : False;
        }
        else
            return False;
    }
    else
        yieldError(undefinedOperation);

    return False;
}
Bool handleLabel(char *labelName, char *nextToken, char *line)
{
    Bool isValid = True;
    if (!labelName || !nextToken || !line)
        return False;
    if (isInstruction(nextToken))
    {
        int instruction = getInstructionType(nextToken);
        if (!isInstructionStrict(nextToken))
        {
            isValid = yieldError(missinSpaceAfterInstruction);
            nextToken = getInstructionNameByType(instruction);
        }

        if (instruction == _TYPE_ENTRY || instruction == _TYPE_EXTERNAL)
        {
            char *next = strtok(NULL, " \t\n\f\r");
            if (next)
                return handleInstruction(instruction, nextToken, next, line) && isValid;
            else
                return yieldWarning(emptyLabelDecleration);
        }
        else
            return handleInstruction(instruction, labelName, nextToken, line) && isValid;
    }

    else if (isOperation(nextToken))
    {
        int icAddr = getIC();
        char args[MAX_LINE_LEN] = {0};
        int offset = (int)(strlen(labelName) + strlen(nextToken) + 1);
        strcpy(args, &line[offset]);
        if (handleOperation(nextToken, args))
            return addSymbol(labelName, icAddr, 1, 0, 0, 0) ? True : False;
        else
            return False;
    }

    else
        yieldError(illegalLabelUseExpectedOperationOrInstruction);

    return False;
}
/* --- END OF ./firstRun.c --- */


/* --- START OF ./headers/functions/compiler.h --- */


/*
##########################################################################################################
##########################################################################################################
#################################### compiler.c General Overview: ########################################
##########################################################################################################
##########################################################################################################
Main file that includes the main function, it gets the files input from the user and then the handleSingleFile
is executing the full proccess of the assembler for each file.
##########################################################################################################
##########################################################################################################
##########################################################################################################
*/
void handleSingleFile(char *arg);
int handleSourceFiles(int argc, char *argv[]);
/* --- END OF ./headers/functions/compiler.h --- */


/* --- START OF ./headers/functions/errors.h --- */


/*
##########################################################################################################
##########################################################################################################
#################################### errors.c General Overview: ##########################################
##########################################################################################################
##########################################################################################################
This file is used for handling different errors our assembler might encounter during all the stages of the
assembler. the errors and printed to the stderr and to a errors.log file that will be created in the root
directory in which the terminal is currently on.
##########################################################################################################
##########################################################################################################
##########################################################################################################
*/

Bool yieldError(Error err);
void yieldErrorIntoFile(Error err);
Bool yieldWarning(Warning err);
void yieldWarningIntoFile(Warning err);
void fileOpeningFailure(char *fileName);
void fileCreationFailure(char *fileName);
void closeOpenLogFiles();

/* --- END OF ./headers/functions/errors.h --- */


/* --- START OF ./headers/functions/exportFiles.h --- */



/*
##########################################################################################################
#################################### exportFiles.c General Overview: #####################################
##########################################################################################################
This file is used for generating the required output files of the assembler.
##########################################################################################################
##########################################################################################################
*/

/*  void exportFilesMainHandler()
This function calls other functions that will create the .ob file,
.ent file and .ext file if there are any entry labels, and .ext file
if there are any external labels.*/
void exportFilesMainHandler();

/* void generateObFile()
This function generates the .ob files by writing the memory image,
if the file can't be generate it yields error  */
void generateObFile();

/* void createEntriesFile()
This function generates the .ent files (as long as there are entry labels) by writing
it into the file, if the file can't be generate it yields error */
void createEntriesFile();

/* createExternalsFile()
This function generates the .ext files (as long as there are external labels)
by writing it into the file, if the file can't be generate it yields error */
void createExternalsFile();
/* --- END OF ./headers/functions/exportFiles.h --- */


/* --- START OF ./headers/functions/firstRun.h --- */


/*
   ##########################################################################################################
   ###########################################  General Overview: ###########################################
   ##########################################################################################################

   In FirstRun.c we are parsing and analysizing the assembly code we got after we extracted the macros from it
   we verify that the syntax is legal and that the use of the assembly language follows the rules of our
   operations and instructions (operands type match, instruction type match, comment type match, legal declaretion
   of symbols, etc). If we encounters error we yields (print to error.log file and to stderr) the specific error and
   we continue checking next characters in line. For each line of code.

   If no errors encountered then we will only INCREASE the DATA COUNTER or the INSTRUCTION COUNTER
   correspondingly by the size that the line of code should take in memory (we calculate the size
   of each instruction or operation in this proccess as well).

   By doing so, at the end of the first run we will know exacly how much space we will need for the data image and how
   much space we need for the instruction image. Doing so allows us to allocate just the required amount of memory
   needed without wasting big random chuncks of memory upfront. If everything goes well, means that the  first Run end
   without errors, then we set the sharedState ((*globalState)()) to secondRun.

   ##########################################################################################################
   ##########################################################################################################
   ##########################################################################################################
   ##########################################################################################################

 */

/* @ Function: handleOperation
   @ Arguments: The function gets char *operationName which the name of the current operation and char * args which are the arguments of the operation.
   @ Description:This function is the main function of parsing operations. It splits the operands from the line, it calls to the functions to validate them.
   For instance, it calls to parseOperand to validate that all of the operation are legal.
   */
/* @ Function: handleOperation
   @ Arguments:
   @ Description:
   */
Bool handleOperation(char *operationName, char *args);
/* @ Function: handleOperation
 @ Arguments: This function gets the source and destination operands, the operation and the posible addresing method of each of the operands.
 @ Description: The function checks if the source and destination operands are valid, by checking if it has the right amount of operands, or the right addressing method.
  If the function encounter errors, it yields (prints) the relevant error message and keeps on parsing the rest of the line in order to find and report all the errors.
  Returns true if the operands are valid, and false if they aren't.
 */

Bool parseOperands(char *src, char *des, const Operation *op, AddrMethodsOptions active[2]);
/* @ Function: validateOperandMatch
   @ Arguments: This function gets AddrMethodsOptions allowedAddrs- the allowed addressing methods for the source and destination operands, AddrMethodsOptions active[2]- an array that represent types of addressing methods for the operands,
    char *operand- the operands, and type- the type of the operand.
   @ Description: The function checks if the operand has the right addressing mrhod, returns true if it does, and false if it doesn't.
   */
Bool validateOperandMatch(AddrMethodsOptions allowedAddrs, AddrMethodsOptions active[2], char *operand, int type);

Bool handleInstruction(int type, char *firstToken, char *nextTokens, char *line);
/* @ Function: Bool handleInstruction(int type, char *firstToken, char *nextTokens, char *line)
   @ Arguments: the function gets int type- the type of the instruction,
   char *firstToken and char *nextToken- the first and second tokens in the instruction line (char * line).
   @ Description: The function checks the first token. If it is an instruction,
   it checks what is the type of the instruction, and yeilds error if there are illegal appearence of characters in the declaration.
   If it is a label declaration, it makes sure there is a valid instruction
   and its arguments afterwards, if the declaration is not valid it yeilds error.
   If the firstToken is not a label declaration or any type of legal instruction, it yeilds error.
   The function keeps on validating the whole line and reports all of the errors.
   If there aren't any mistake, the function returns true and if there is a vald label it adds it to the symbols table.
   Else, it returns false.

*/

Bool handleLabel(char *labelName, char *nextToken, char *line);
/*  @ Function: Bool handleLabel(char *labelName, char *nextToken, char *line)
    @ Arguments: The function gets char *labelName- the name of the label it handles,
    char *nextToken- one of the tokens in the line (char * line).
    @ Description: The function gets a line in the code that starts with a label declaration.
    The function checks if this is an instruction line or an operation line.
    If it encountered an error it yields (prints) the error.
     The function keeps on validating the whole line and reports all of the errors.
    The function returns true if the line is valid, andfalse if it ian't.
*/
/* --- END OF ./headers/functions/firstRun.h --- */


/* --- START OF ./headers/functions/functions.h --- */

#include "sharedStates.h"
#include "compiler.h"
#include "operation.h"

#include "utils.h"
#include "errors.h"
#include "helpers.h"
#include "tables.h"
#include "preProccesor.h"
#include "parse.h"
#include "memory.h"
#include "firstRun.h"
#include "secondRun.h"
#include "exportFiles.h"
/* --- END OF ./headers/functions/functions.h --- */


/* --- START OF ./headers/functions/helpers.h --- */


/*
##########################################################################################################
##########################################################################################################
#################################### helpers.c/h General Overview: ########################################
##########################################################################################################
##########################################################################################################

this file containing some helper functions that are used in other files for doing simple and independent small
tasks like cloning a string, trimming white space, and also it contains helpers functions that are used for
encoding numeric values in binary(2's complement)/hex string.

##########################################################################################################
##########################################################################################################
##########################################################################################################
*/

/* @ Function: char *cloneString(char *s)
   @ Arguments: The function gets char *s- the string it copies
   @ Description: The function gets a char *s, if it is equal to NULL, it returns empty string.
   Else it copies the string to a new memory allocation and returns it.
*/
char *cloneString(char *s);

/* @ Function: char *trimFromLeft(char *s)
   @ Arguments: The function gets char *s- the string it trims from the white spaces from the left.
   @ Description: The function gets a char *s, while it hasn't reached the string's end,
   it skips all the white-space characters that are in the begining of this string
   and than returns a pointer to the string
   (which points to the part after the white space that were before).
*/
char *trimFromLeft(char *s);

char *decToHex(int num);
char *numToBin(int num);
HexWord *convertBinaryWordToHex(BinaryWord *word);
unsigned binaryStringToHexNumber(char binaryStr[4]);
/* --- END OF ./headers/functions/helpers.h --- */


/* --- START OF ./headers/functions/memory.h --- */


/*
##########################################################################################################
##########################################################################################################
#################################### memory.c/h General Overview: ########################################
##########################################################################################################
##########################################################################################################

This memory purpose is for managing the memory of the resulted binary/hex image, it is used in the first
run and the second run as well. in the first run functions in here used to increase and decrease the DC/IC
counters and in the second run the function included here builds the memory image word by word.

##########################################################################################################
##########################################################################################################
##########################################################################################################
*/

void increaseDataCounter(int amount);
/* @ Function: increaseDataCounter
   @ Arguments: The function gets int amount- the amount of addresses in the memory the data counter holds.
   @ Description: The function increases the data counter by amount- the amount of addresses in the memory the data counter holds.
   The function doesn't return any value.
*/

void increaseInstructionCounter(int amount);
/* @ Function: increaseInstructionCounter
   @ Arguments: The function gets int amount- the amount of addresses in the memory the instruction counter holds.
   @ Description: The function increases the instruction counter by amount- the amount of addresses in the memory the instruction counter holds.
   The function does'nt return any value.
*/

void allocMemoryImg();
/* @ Function: allocMemoryImg
   @ Arguments: The function gets no arguments.
   @ Description: The function allocates the exact amount of space required to generate the compiled image.
   The function doesn't return any value.
*/
void resetMemoryCounters();
/* @ Function: resetMemoryCounters
   @ Arguments: The function gets no arguments.
   @ Description: The function reset the counter that counts the exact amount of space required to generate the compiled image.
   The function doesn't return any value.
*/

void wordStringToWordObj(char *s, DataType type);
/* @ Function: wordStringToWordObj
   @ Arguments: The function gets char *s - the string it converts and DataType type- tells us if the current word belongs to the instructions or to the tha data part of the memory image.
   converts a string from a binary word to typedef struct
   @ Description: The function converts a string to a binaryWord typedef struct (defines in complex_typedef.h).
   The function doesn't return any value.
*/

void writeMemoryImageToObFile(FILE *fp);
/* @ Function: writeMemoryImageToObFile
   @ Arguments: The function gets FILE *fp the file it writes the ob memory into
   @ Description: The function generates the .ob memory image of the code. It writes into the .ob file word by word using helper functions to convert binary word into an hex word.
   The function doesn't return any value.
*/

void printMemoryImgInRequiredObjFileFormat();
void calcFinalAddrsCountersValues();
void printWordBinary(unsigned index);
void addWordToCodeImage(char *s);
void addWordToDataImage(char *s);
void addWord(int value, DataType type);
void printBinaryImg();

unsigned getDC();
unsigned getIC();
unsigned getICF();
unsigned getDCF();
/* --- END OF ./headers/functions/memory.h --- */


/* --- START OF ./headers/functions/operation.h --- */


/*
##########################################################################################################
##########################################################################################################
#################################### Operation.c/h General Overview: #####################################
##########################################################################################################
##########################################################################################################

This file holds the operation table information and contain functions that their purpose is to share different
parts of the operation information from this table.

##########################################################################################################
##########################################################################################################
##########################################################################################################
*/

Bool isOperationName(char *s);
const Operation *getOperationByName(char *s);
const Operation *getOperationByIndex(unsigned int i);
const char *getOperationName(char *s);
int getOpIndex(char *s);
Bool isOperationNotStrict(char *s);

/* --- END OF ./headers/functions/operation.h --- */


/* --- START OF ./headers/functions/parse.h --- */


/*
##########################################################################################################
##########################################################################################################
#################################### preProccesor.c/h General Overview: ##################################
##########################################################################################################
##########################################################################################################

This file purpose is to be the main file that is responsible for parsing of the expanded assembly code.
while parsing the code in this file we are invoking different functions that exists in the firstRun.c
and the secondRun.c depending on the global state of our assembler (firstRun/SecondRun).

##########################################################################################################
##########################################################################################################
##########################################################################################################
*/

/* @ Function: verifyCommaSyntax
   @ Arguments: char * which will hold the content of some line of code.
   @ Description:The function validates the comma syntax in some line of code,
   it checks that between any 2 arguments their exist only one comma and that
   there are not commas in the end or the beginning of the line. If a line
   found to be violating these comma syntax rules, it yields the relevant error
   message.
*/

Bool verifyCommaSyntax(char *line);

/* @ Function: countAndVerifyDataArguments
   @ Arguments: the function get char * line which is the current line that we are about to parse the data arguments from.
   @ Description: The function extracts the argument string of the .data instruction, than the function analyses\ parses the string.
   If the function encounter errors, an illegal comma or illegal arguments, it yields (prints) the relevant error message and keeps on parsing the rest of the line in order to find and report all the errors.
   While the function parsing the arguments, ir also counts the number of .data elements that will take size in the data memory.
   In the end of the function, if after parsing the line turns out to be valid, it increases the data counter with the size in memory that the current .data instruction will take.
*/

Bool countAndVerifyDataArguments(char *line);

/* @ Function: countAndVerifyStringArguments
   @ Arguments: the function gets char * token which is the current token that we are about to parse the string argument from.
   @ Description: The function extracts the argument string of the .string instruction, than the function analyses\ parses the string.
   If the function encounter errors- no opening or closing  (or no opening and closing) quotes, it yields (prints) the relevant error message and keeps on parsing the rest of the line in order to find and report all the errors.
   While the function parsing the arguments, ir also counts the length of the .string string (including the \0 at the end) that will take size in the data memory.
   In the end of the function, if after parsing the line turns out to be valid, it increases the data counter with the size in memory that the current .string instruction will take.
*/
Bool countAndVerifyStringArguments(char *line);

/* @ Function: parseLine
   @ Arguments: The function gets char * token which is the first token of the line that we are about to parse and char *line which is the current line being parsed
   @ Description: The function checks what is the current globalState, than checks what is the first token (an instruction, an operation, a label declaration...)
    If the globalState is equal to firstRun, according to the first token, it finds out what kind of line is the current line and calls other function so they can handle the line.
    If the globalState is equal to secondRun, according to the first token, it calls function to write the compiled memory.
    If it finds some errors, it yields (prints) the relevant error message.
    If the line turns out to be valid, it returns true, else it returns false.
*/
Bool parseLine(char *token, char *line);

/* @ Function:handleSingleLine
   @ Arguments: This function gets char *line which is the current line that is being parsed.
   @ Description: The function checks what is the current globalState, and by that decides how to split the line into tokens (what is the delimeter).
   Than the function calls the function parseLine in order to check if there are errors in the line.
   Than its increases by 1 the number of line it parses (for the next line to come).
   Returns true if the line is valid and false if it isn't.
*/
Bool handleSingleLine(char *line);

/* @ Function:parseAssemblyCode
   @ Arguments: The function gets FILE *src which is a file after the pre proccesoring.
   @ Description: The function parses the file, splits it to tokens and calls the right functions to complete the first and second run.
*/
void parseAssemblyCode(FILE *src);

/* --- END OF ./headers/functions/parse.h --- */


/* --- START OF ./headers/functions/preProccesor.h --- */


/*
##########################################################################################################
##########################################################################################################
#################################### preProccesor.c/h General Overview: ##################################
##########################################################################################################
##########################################################################################################

The pre-proccessor is for parsing the original .as file and replacing macros if existing using hash table to
store the starting and ending index of each macro in file. if we found an error in the syntax of the macros
we yield relevant error message and will not continue to the first run.

##########################################################################################################
##########################################################################################################
##########################################################################################################
*/

/* @ Function: void parseSourceFile(FILE *src, FILE *target)
   @ Arguments: FILE *src, FILE *target. the *src FILE pointer
   is a file pointer to the original source code file (".as" file) and the *target FILE
   pointer argument is for an empty file that will be the expanded source file version of the src
   file without the macros (target is the ".am" file we will generate from source in this function);
   @ Description: This function splits the source file content to different lines, and it pass each
   seperated line to the parseMacros(char *line, char *token, FILE *src, FILE *target)
*/
void parseSourceFile(FILE *src, FILE *target);

/* @ Function: void parseMacros(char *line, char *token, FILE *src, FILE *target);
   @ Arguments: char *line, char *token, FILE *src, FILE *target
   *src and *target FILE pointers are the same as in the function we mentioned above.
   line is the copy of the current line we parsed from the source file and token is the
   first non-empty space token found in that line.
   @ Description: this function is invoked by parseSourceFile function and is in take write the
   line into the new ".am" expanded file after checking if there are macros on line (macro opening/closing/mentioning)
   and ignoring/replacing them correspondingly.
*/
void parseMacros(char *line, char *token, FILE *src, FILE *target);

/* --- END OF ./headers/functions/preProccesor.h --- */


/* --- START OF ./headers/functions/secondRun.h --- */


/*
##########################################################################################################
##########################################################################################################
#################################### secondRun.c General Overview: #######################################
##########################################################################################################
##########################################################################################################

In the second run our assembler writes the words of each line of code in binary format, we inserting the words to
the memory image in the correct position depending on the type of word (data/instruction) and we add each external
operand appearence to the linked list of all the positions that external operands appeared. if we encouter label
operand that is not within the symbol table and is not external we yield error message and continue checking the
rest of the code in order to discover all errors of this type if this case happens we will finish second run but
will not export any files.

##########################################################################################################
##########################################################################################################
##########################################################################################################
*/

Bool writeOperationBinary(char *operationName, char *args);
/* @ Function: writeOperationBinary
   @ Arguments: The function gets char *operationName- the name of the operation and char *args - the arguments of the operation
   @ Description: The function splits char * args to tooken- each token is a different operand.
   Then the function checks what is the kind of the operands and writes them in words at the memory.
   If there is an operand which is an undifined labl at the table of symbols, it returns false. Else it returns true.
*/

void writeAdditionalOperandsWords(const Operation *op, AddrMethodsOptions active, char *value);
/* @ Function: writeAdditionalOperandsWords
   @ Arguments: The function gets const Operation *op- the operation, AddrMethodsOptions active- the allowed addresing methods for the operands,
   and char * value- the operand which is about to be written in the memory.
   @ Description: The functiom checks the addressing method of the operations, then acording to the addressing method it writes value (the operand) in the memory.
    The function doesn't return value.
*/

Bool writeDataInstruction(char *token);
/* @ Function: writeDataInstruction
   @ Arguments: The function gets char *token, which is the token of the .data instruction's arguments.
   @ Description: The function spilts char *token into tokens- separate each one of the .data arguments, then it adds and writes them in the memory.
   The function returns true.
*/

Bool writeStringInstruction(char *s);
/* @ Function: writeStringInstruction
   @ Arguments: The function gets char *s, which is the string of the .string instruction.
   @ Description: The function search the opening quotes in char *s, to find out where the string starts.
   Then, it it adds and writes the characters of the string in the memory.
   The function returns true.
*/

void writeSecondWord(char *first, char *second, AddrMethodsOptions active[2], const Operation *op);
/* @ Function: writeSecondWord
   @ Arguments: The function gets char *first and char *second - the first and second operands of the operation- const Operation *op.
   It also gets AddrMethodsOptions active[2]- an array with the allowed addresing methods for the operands.
   @ Description: The function builds and writes in the memory the second word of each operation and its parameters, it checks what is the addressing method of first and second, what is the funct of the operation.
    The function doesn't return value.
*/

void writeFirstWord(const Operation *op);
/* @ Function: writeFirstWord
   @ Arguments: The function gets const Operation *op- the operation it writes uts first word.
   @ Description: The function writes in the memory the first word of the operation and its parameter by writing the opcode of the operation into the memory.
    The function doesn't return value.
*/

void writeDirectOperandWord(char *labelName);
/* @ Function: writeDirectOperandWord
   @ Arguments: The function gets char *labelName which is the label it will update the the external position table.
   @ Description: The function updates the external operands position table that we will gnerate from the .ext files.
    The function doesn't return value.
*/

void writeImmediateOperandWord(char *n);
/* @ Function: writeImmediateOperandWord
   @ Arguments: The function gets char *n- the immmediate operand it will add to the memory.
   @ Description: The function adds the immediate operand's numeric value by using the function addWord which will write it in the memory.
   The function doesn't return value.
*/

Bool detectOperandType(char *operand, AddrMethodsOptions active[2], int type);
/* @ Function: detectOperandType
   @ Arguments: The function gets char *operand- the operand it checks it's type, AddrMethodsOptions active[2]- an array with the allowed addresing methods for the operand, and int type- the type of the operand (source or destination).
   @ Description: The function checks what is the addressing method for the operand.
    If it is a label (direct parameter), it checks if it is exist in the table symbol (if not it yields error) and if the operand is both entry and externl which is illegal, and then it yeilds (prints) error too.
    Returns true if the operation is valid, and false if it isn't.
*/

char *parseLabelNameFromIndexAddrOperand(char *s);
/* @ Function: parseLabelNameFromIndexAddrOperand
   @ Arguments: The function gets char *s, which is the operand from index addressing method it parses.
   @ Description: The function parses out the label name from the char *s it gets.
   Returns the label name from the token it got.
*/

int parseRegNumberFromIndexAddrOperand(char *s);
/* @ Function: parseRegNumberFromIndexAddrOperand
   @ Arguments: The function gets char *s, which is the operand from index addressing method it parses.
   @ Description: The function parses out the register's number from the char *s it gets.
   Returns the register's number from the token it got.
*/
/* --- END OF ./headers/functions/secondRun.h --- */


/* --- START OF ./headers/functions/sharedStates.h --- */


/*
##########################################################################################################
##########################################################################################################
#################################### sharedStates.c/h General Overview: ##################################
##########################################################################################################
##########################################################################################################
the file sharedStates.h/c exist for holding and safely sharing all the global states information about our
program at current moment which are:

1.The current state of our program (parsing macros/first run/etc...),the source file name which is currently
being handled by the assembler.

2.the current original source filename path.

3.the current line we are on while parsing macros/first run/second run/error printing.

The implementation for this is done by making a getters and setters functions to update static global variables
that are defined in the file so they would be closed to its scope.

##########################################################################################################
##########################################################################################################
##########################################################################################################
*/

void setGlobalState(State newState);
/* @ Function: setGlobalState
   @ Aruments: The function gets State new- the state the function will update
   @ Description: The function updates the globalState to be the state it gets. This is the one of the only times we have an access to the global state (only through using the functions setGlobalState and getGlobalState)
    The function doesn't return any value.
*/

State getGlobalState();
/* @ Function: getGlobalState
   @ Aruments: The function doesn't get any arguments.
   @ Description: The function gets the global state. This is the one of the only times we have an access to the global state (only through using the functions setGlobalState and getGlobalState)
    The function returns the current global state.
*/

void setFileNamePath(char *s);
/* @ Function: setFileNamePath
   @ Aruments: The function gets char *s which is a current file name that is handled by our assembler at the moment.
   @ Description: This is a setter function that updates te state of the current file name path, this state is used
   in other files of the program that needs to know what is the base file name and exact path of the current file that is being handled.
    The function doesn't return any value.
*/

char *getFileNamePath();
/* @ Function: getFileNamePath
   @ Aruments: The function doesn't get any arguments.
   @ Description: The function is a getter function for the state of the current file name path.
   The function returns separated copy of the string (clone), in order for other parts of the program
   that are also dealing with creating an outputing different file outputs (.ob, .ext, .ent, yieldError and more)
    for each file input the program gets fromthe user.
*/

void resetCurrentLineNumber();
/* @ Function: resetCurrentLineNumber
   @ Aruments: The function doesn't get any arguments.
   @ Description: The function resets currentLineNumber to be 1.
    The function doesn't return any value.
*/

void increaseCurrentLineNumber();
/* @ Function: increaseCurrentLineNumber
   @ Aruments: The function doesn't get any arguments.
   @ Description: The function increases currentLineNumber by 1.
    The function doesn't return any value.
*/

int getCurrentLineNumber();
/* @ Function: getCurrentLineNumber
   @ Aruments: The function doesn't get any arguments.
   @ Description: The function increases currentLineNumber by 1.
    The function doesn't return any value.
*/
/* --- END OF ./headers/functions/sharedStates.h --- */


/* --- START OF ./headers/functions/tables.h --- */


/*
##########################################################################################################
##########################################################################################################
#################################### tables.c General Overview: ##########################################
##########################################################################################################
##########################################################################################################

The file tables.c is responsible of all the tables, lists and data structures in the code.
It handles the macros table, the symbols table and the external operands nested singly linked list
Both the symbols and the macros able are using the same hash table data structure and main function\interface.
The hashtable main functions are lookup hash and install for finding \ adding \ hashing items in the symbol table.
Each type of table (symbol\macro\external operand) have additional helpers, getters and setters functions that are
using the 3 hash table main functions (install\lookup\hash).

##########################################################################################################
##########################################################################################################
##########################################################################################################
*/

/*#####################################################################*/
/*#####################################################################*/
/*####################### Hash Table Functions ############################*/
/*#####################################################################*/
/*#####################################################################*/

/* @ Function: unsigned hash(char *s)
   @ Arguments: The function gets char *s- a label name
   @ Description: The function gets a string that is a label name,
   it hashes it with a simple algorithem and
   it returns the index position in the macros/symbols hash table array.
*/
unsigned hash(char *s);
/* @ Function: Item *lookup(char *s, ItemType type)
   @ Arguments: char *s (label name) ItemType type (Macro/Symbol)
   @ Description: Function finds the item with the name and of the type
   in the correspoding hash table (macros or symbols hash table)
   it returns a pointer to a typedef struct Item;
*/
Item *lookup(char *s, ItemType type);

/* @ Item *install(char *name, ItemType type)
   @ Arguments:char *s (label name) ItemType type (Macro/Symbol)
   @ Description: Function adds the label name to the corresponding hash table
   it uses hash function and if the index of the hash table is already in use
   thne the it connects the new entry to be the *next of the item under the common index.
   (symbols/macros tables acts as singly linked list within the hash table in those fcases)
   it returns a pointer to a typedef struct Item for the new Item added to the table;
*/
Item *install(char *name, ItemType type);

/* void initTables();
void freeHashTable(ItemType type);
void freeTableItem(Item *item); */

/*#####################################################################*/
/*#####################################################################*/
/*##########################Symbols functions#############################*/
/*#####################################################################*/
/*#####################################################################*/

Bool addSymbol(char *name, unsigned value, unsigned isCode, unsigned isData, unsigned isEntry, unsigned isExternal);
Bool updateSymbol(Item *p, unsigned value, unsigned isCode, unsigned isData, unsigned isEntry, unsigned isExternal);
Item *getSymbol(char *name);
int getSymbolBaseAddress(char *name);
int getSymbolOffset(char *name);
Bool isSymbolExist(char *name);
Bool isExternal(char *name);
Bool isEntry(char *name);
Bool isNonEmptyExternal(char *name);
Bool isNonEmptyEntry(char *name);
Bool isLabelNameAlreadyTaken(char *name, ItemType type);
Item *updateSymbolAddressValue(char *name, int newValue);
void updateFinalSymbolTableValues();
void updateFinalValueOfSingleItem(Item *item);
Bool areEntriesExist();
Bool areExternalsExist();
void printSymbolTable();
int printSymbolItem(Item *item);
void writeEntriesToFile(FILE *fp);
int writeSingleEntry(Item *item, FILE *fp, int count);

/*#####################################################################*/
/*#####################################################################*/
/*########################### Macro functions ###########################*/
/*#####################################################################*/
/*#####################################################################*/

Item *addMacro(char *name, int start, int end);
Item *getMacro(char *s);
Item *updateMacro(char *name, int start, int end);
void printMacroTable();
int printMacroItem(Item *item);

/*#####################################################################*/
/*#####################################################################*/
/*############ External Operand Table (Nested singly linked list) ######*/
/*#####################################################################*/
/*#####################################################################*/

ExtListItem *findExtOpListItem(char *name);
void resetExtList();
void updateExtPositionData(char *name, unsigned base, unsigned offset);
void addExtListItem(char *name);
void writeExternalsToFile(FILE *fp);
void writeSingleExternal(FILE *fp, char *name, unsigned base, unsigned offset, ExtPositionData *next);
/* --- END OF ./headers/functions/tables.h --- */


/* --- START OF ./headers/functions/utils.h --- */


/*
##########################################################################################################
##########################################################################################################
#################################### utils.c/utils.h General Overview: ###################################
##########################################################################################################
##########################################################################################################

Utils.c files contains most of all helpers functions that are used for verifying and checking and
validating tokens identity, funtion here are mainly used out side of this file by other functions that deals
with parsing the assembly code.

##########################################################################################################
##########################################################################################################
##########################################################################################################
*/

Bool isMacroOpening(char *s);
/* @ Function: isMacroOpening
   @ Arguments: The function gets a char *s, the string it checks whether it is the opening of a macro or not.
   @ Description: The function checks if char *s is equal to "macro", returns true if it is, and false if it isn't
*/

Bool isMacroClosing(char *s);
/* @ Function: isMacroClosing
   @ Arguments: The function gets a char *s, the string it checks whether it is the closing of a macro or not.
   @ Description: The function checks if char *s is equal to "endm", returns true if it is, and false if it isn't
*/

Bool isPossiblyUseOfMacro(char *s);
/* @ Function: isPossiblyUseOfMacro
   @ Arguments: The function gets a char *s, the string it checks whether it can be a macro or not.
   @ Description: The function checks if char *s can be a macro, by illuminating the other things it can be (an instruction, a label...).
   Returns true if it could be a macro and false if it couldn't.
*/

Bool isLegalMacroName(char *s);
/* @ Function: isLegalMacroName
   @ Arguments: The function gets a char *s, the string it checks whether it is a legal macro name or not.
   @ Description: The function checks if char *s is a legal macro name by verifying that it's name is not a name of an instruction or an operation.
    Returns true if it is a legal macro name and false if it is not.
*/

Bool isInstruction(char *s);
/* @ Function: isInstruction
   @ Arguments: The function gets a char *s, the string it checks whether it or a part of it is an instruction or not.
   @ Description: The function checks if char *s or a part of it is an instruction by comparing it to the name of the instructions or checking if one of the instructions are a substring of it.
   Returns true if it or a part of it is an instruction, and false if it isn't.
*/

Bool isInstructionStrict(char *s);
/* @ Function: isInstructionStrict
   @ Arguments: The function gets a char *s, the string it checks whether it is an instruction or not.
   @ Description: The function checks if char *s is an instruction by comparing it to the name of the instructions.
   Returns true if it is an instruction, and false if it isn't.
*/

Bool isRegistery(char *s);
/* @ Function: isRegistery
   @ Arguments: The function gets a char *s, the string it checks whether it is a register or not.
   @ Description: The function checks if char *s is a legal register by checking if the first char is 'r' and theothers are int numbers between 0-15.
   Returns true if it is a register, and false if it isn't.
*/

Bool isValidImmediateParamter(char *s);
/* @ Function: isValidImmediateParamter
   @ Arguments: The function gets a char *s, the string it checks whether it is a valid immediate parameter or not.
   @ Description: The function checks if char *s is a valid immediate parameter by verifying the first token us a -, a + or a digit and the other vhars are digits.
   Returns true if it is a valid immediate parameter, and false if it isn't.

*/

Bool isValidIndexParameter(char *s);
/* @ Function: isValidIndexParamter
   @ Arguments: The function gets a char *s, the string it checks whether it is a valid index parameter or not.
   @ Description:The function checks if char *s is a valid index parameter by checkin if it is a label and a register (between 10-15) in parenthesis afterwards
   Returns true if it is a valid index parameter, and false if it isn't.
*/

Bool isIndexParameter(char *s);
/* @ Function: isIndexParameter
   @ Arguments: The function gets a char *s, the string it checks whether it is a index parameter or not.
   @ Description:The function checks if char *s is a valid index parameter by checkin if it is a label and a register (any register, not strict to 10-15) in parenthesis afterwards
   Returns true if it is a valid index parameter, and false if it isn't.
*/

Bool isComment(char *s);
/* @ Function: isComment
   @ Arguments: The function gets a char *s, the string it checks whether it is a comment or not.
   @ Description: The function checks if char *s is a comment by checking if it's first char is ;.
   Returns true if it is a comment, and false if it isn't.
*/
Bool isOperation(char *s);
/* @ Function: isOperation
   @ Arguments: The function gets a char *s, the string it checks whether it is an operation or not.
   @ Description: The function checks if char *s is an operation, by checking if it is equal to one of the names of the operations, using getOperationByName.
   Returns true if it is an operation, and false if it isn't.
*/

Bool isLabelDeclarationStrict(char *s);
/* @ Function: isLabelDeclarationStrict
   @ Arguments: The function gets a char *s, the string it checks whether it is a label declaration or not.
   @ Description: The function checks if char *s is a label declaration by checking if it's last char is :.
     Returns true if it is a label declaration, and false if it isn't.
*/

Bool isLabelDeclaration(char *s);
/* @ Function: isLabelDeclaration
   @ Arguments: The function gets a char *s, the string it checks whether it or a part of it is a label declaration or not.
   @ Description: The function checks if char *s or a part of it is a label declaration by checking if it consists a :.
   Returns true if it or a part of it is a label declaration, and false if it isn't.
*/

int getInstructionType(char *s);
/* @ Function: getInstructionType
    @ Arguments: The function gets a char *s, the string it checks what instruction it is.
    @ Description: The function checks if char *s or a part of it is an instruction  by  checking if one of the instructions are a substring of it.
    Returns the right instruction if it is an instruction, and false(0) if it isn't
*/

int getRegisteryNumber(char *s);
/* @ Function: getRegisteryNumber
    @ Arguments: The function gets a char *s, the string it checks from what is the number of the register.
    @ Description: The function checks what is the number of the register by checking the number at the end of the string
    Returns the number of the register.
*/

char *getInstructionName(char *s);

char *getInstructionNameByType(int type);
/* @ Function: getInstructionNameByType
   @ Arguments: The function gets int type- the type of instruction.
   @ Description: The function returns the type of the istruction and returns it as a string with \0 at the end.
   If type is not an instruction it returns NULL.
*/

Bool verifyLabelNaming(char *s);
/* @ Function: verifyLabelNaming
   @ Arguments: The function gets char *s- the name of the label it about to verify.
   @ Description: The function makes sure the name of the label is valid, by checking things like its length, if it is equal to an operation...
   Returns true if it is a valid name of label, and false if it isn't.
*/

Bool verifyLabelNamingAndPrintErrors(char *s);
/* @ Function: verifyLabelNamingAndPrintErrors
   @ Arguments: The function gets char *s- the name of the label it about to verify and prints it's errors.
   @ Description: The function makes sure the name of the label is valid, by checking things like its length, if it is equal to an operation...
   If it finds an error it yields (prints) it and returns false, else returns true.
*/

/* --- END OF ./headers/functions/utils.h --- */


/* --- START OF ./headers/lib/lib.h --- */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>
#include <stdarg.h>

/* --- END OF ./headers/lib/lib.h --- */


/* --- START OF ./headers/variables/complex_typedef.h --- */

#include "constants.h"

/*Bit is is a special variable that we made to build with BinaryWord*/
typedef struct
{
    unsigned int on : 1;
} Bit;
/*BinaryWord is a struct for holding memory and data image in memory format*/
typedef struct
{
    Bit digit[BINARY_WORD_SIZE];
} BinaryWord;

/*Like BinaryWord, it represent a word with 5 hexa decimal characters in the order we will output to th .ob file*/
typedef struct
{
    unsigned int _A : 4;
    unsigned int _B : 4;
    unsigned int _C : 4;
    unsigned int _D : 4;
    unsigned int _E : 4;
} HexWord;

/*AddrMethodsOptions is a struct for addressing mehod, each member represent operand addressing method
and the value can be 0 or 1 and the meaning is to  tell us if the addressing method is legal/optional
For each operation's operand that we have in our assembly language.
*/
typedef struct
{
    unsigned int immediate : 1;
    unsigned int direct : 1;
    unsigned int index : 1;
    unsigned int reg : 1;
} AddrMethodsOptions;

/*Operation is a struct for holding all the operation table data*/
typedef struct
{
    unsigned int op;        /*op- is the binary value of the first sixteen bits in the first word for each operation*/
    unsigned int funct : 4; /*the function code of the operation*/
    char keyword[4];        /*the keyword for the operation name*/
    AddrMethodsOptions src; /*legal addressing mehods for the source operand*/
    AddrMethodsOptions des; /*legal addressing mehods for the destination operand*/
} Operation;

/*ItemType is an identyfier parameter that we use in the hash table functions that both the macro table and the symbol table are built with*/
typedef enum
{
    Macro,
    Symbol
} ItemType;
/*DataType is an identyfier parameter that we use in the memory.c file functions in order to specify if a current memory word belongs to the data
image part or the instruction part of the resulted image*/
typedef enum
{
    Code,
    Data
} DataType;

/* MacroData is a struct that holds the additional unique data fields of each macro item in the macros hash table. The start member represents the starting index of the content of the macro in the source (.as) files
 The end member is the same as start, but for the ending of the macro content*/
typedef struct
{
    int start;
    int end;
} MacroData;

/* ExtPositionData is the nested singly linked list element in the ExtListItem that we just mentioned above.*/

typedef struct ExtPositionData
{
    unsigned base;
    unsigned offset;
    struct ExtPositionData *next;
} ExtPositionData;

/* ExtListItem is a typedef struct of a single external symbol name
that is used as operand in the assembly code. This item is the building block
of the external operands nested singly linked list. The nested list is a singly linked list
that holds ExtPositionData (base, offset) for any memory address the
external operand appeares to be used in the binary / hexa image we create. */
typedef struct ExtListItem
{
    char name[MAX_LABEL_LEN];
    ExtPositionData value;
    struct ExtListItem *next;
} ExtListItem;

/*Attributes for the type of symbol that are saved in the ymbol table*/
typedef struct
{
    unsigned int code : 1;
    unsigned int data : 1;
    unsigned int entry : 1;
    unsigned int external : 1;
} Attributes;

/*SymbolData is a struct that holds the additional unique data fields in the symbols hash table.
 The value member represents the  value of the memory address that the symbol appearse in.
The base and offset members represent the same address but in format that splits the memory address to
base address and offset address.
The attrs member is holding the type of the symbol (data, entry, external, code).
 */
typedef struct
{

    unsigned value;
    unsigned base;
    unsigned offset;
    Attributes attrs;
} SymbolData;

/*Item is the building block of the hash table data structure that both the symbols and macros tables are built with*/
typedef struct Item
{
    char *name;
    union
    {
        SymbolData s;
        MacroData m;
    } val;
    struct Item *next;
} Item;

/* --- END OF ./headers/variables/complex_typedef.h --- */


/* --- START OF ./headers/variables/constants.h --- */


#define BINARY_WORD_SIZE 20
#define MEMORY_START 100
#define REGS_SIZE 16
#define OP_SIZE 16
#define RAM_MEMORY_SIZE 8192
#define MAX_LABEL_LEN 31
#define MAX_LINE_LEN 81

/* Hash Table Hashsize (Result is currently 64)  */
#define HASHSIZE ((RAM_MEMORY_SIZE / (MAX_LABEL_LEN + 1)) / 4)

/* Masks for creation of to binary word memory image */
#define A 0x0004
#define R 0x0002
#define E 0x0001
#define IMMEDIATE_ADDR 0x00
#define DIRECT_ADDR 0x01
#define INDEX_ADDR 0xA
#define REGISTER_DIRECT_ADDR 0xB

#define R0 0x0
#define R1 0x1
#define R2 0x2
#define R3 0x3
#define R4 0x4
#define R5 0x5
#define R6 0x6
#define R7 0x7
#define R8 0x8
#define R9 0x9
#define R10 0xA
#define R11 0xB
#define R12 0xC
#define R13 0xD
#define R14 0xE
#define R15 0xF

#define DATA ".data"
#define STRING ".string"
#define ENTRY ".entry"
#define EXTERNAL ".extern"
/* Given just random Numbers that will be different then 0*/
#define _TYPE_DATA 11
#define _TYPE_STRING 22
#define _TYPE_ENTRY 33
#define _TYPE_EXTERNAL 44
#define _TYPE_CODE 55
/* --- END OF ./headers/variables/constants.h --- */


/* --- START OF ./headers/variables/flags.h --- */


typedef enum
{
    undefinedTokenNotOperationOrInstructionOrLabel,
    illegalLabelDeclaration,
    entryDeclaredButNotDefined,
    useOfNestedMacrosIsIllegal,
    memoryAllocationFailure,
    undefinedOperation,
    undefinedInstruction,
    tooMuchArgumentsPassed,
    notEnoughArgumentsPassed,
    symbolDoesNotExist,
    macroDoesNotExist,
    wrongArgumentTypePassedAsParam,
    illegalMacroNameUseOfCharacters,
    illegalLabelNameUseOfCharacters,
    illegalLabelNameUseOfSavedKeywords,
    illegalLabelUseExpectedOperationOrInstruction,
    illegalLabelNameLength,
    illegalMacroNameLength,
    illegalSymbolNameAlreadyInUse,
    symbolCannotBeBothCurrentTypeAndRequestedType,
    illegalOverrideOfLocalSymbolWithExternalSymbol,
    labelNameAlreadyInUseInsideSymbolTable,
    illegalMacroNameAlreadyInUse,
    macroClosingWithoutAnyOpenedMacro,
    wrongArgumentTypeNotAnInteger,
    wrongInstructionSyntaxMissinCommas,
    wrongInstructionSyntaxExtraCommas,
    wrongInstructionSyntaxMissinQuotes,
    wrongOperationSyntaxMissingCommas,
    wrongOperationSyntaxExtraCommas,
    illegalApearenceOfCharactersOnLine,
    illegalApearenceOfExtraCharactersOnLine,
    illegalApearenceOfCharactersInTheEndOfTheLine,
    illegalApearenceOfCommaBeforeFirstParameter,
    illegalApearenceOfCommaAfterLastParameter,
    missinSpaceAfterInstruction,
    missingSpaceBetweenLabelDeclaretionAndInstruction,
    maxLineLengthExceeded,
    illegalInputPassedAsOperandSrcOperand,
    illegalInputPassedAsOperandDesOperand,
    none,
    noErrors,
    expectedNumber,
    expectedSingleCommaCharacter,
    expectedBlank,
    illegalOverrideOfExternalSymbol,
    AssemblerDidNotGetSourceFiles,
    expectedQuotes,
    closingQuotesForStringIsMissing,
    fileCouldNotBeOpened,
    wrongRegisteryReferenceUndefinedReg,
    requiredSourceOperandIsMissin,
    requiredDestinationOperandIsMissin,
    srcOperandTypeIsNotAllowed,
    desOperandTypeIsNotAllowed,
    operandTypeDoNotMatch,
    labelNotExist,
    wrongInstructionSyntaxIllegalCommaPosition,
    secondRunFailed,
    illegalMacroNameUseOfSavedKeywords,
    afterPlusOrMinusSignThereMustBeANumber,
    illegalApearenceOfCharacterInTheBegningOfTheLine,
    illegalLabelNameUseOfSavedKeywordUsingOperationName,
    illegalLabelNameUseOfSavedKeywordUsingRegisteryName,
    macroDeclaretionWithoutDefiningMacroName,
    wrongCommasSyntaxExtra,
    wrongCommasSyntaxMissing,
    wrongCommasSyntaxIllegalApearenceOfCommasInLine,
    extraOperandsPassed,
    emptyStringDeclatretion,
    emptyLabelDecleration,
    undefinedLabelDeclaretion,
    registeryIndexOperandTypeIfOutOfAllowedRegisteriesRange

} Error;

typedef enum
{
    emptyDeclaretionOfEntryOrExternalVariables,

    emptyEntryDeclaretion,
    emptyExternalDeclaretion,
    instructionHasNoArguments,
    emptyDataDeclaretion

} Warning;

typedef enum
{
    assemblyCodeFailedToCompile,
    startProgram,
    parsingMacros,
    firstRun,
    secondRun,
    exportFiles,
    exitProgram
} State;

typedef enum
{
    False = 0,
    True = 1
} Bool;
/* --- END OF ./headers/variables/flags.h --- */


/* --- START OF ./headers/variables/variables.h --- */


#include "flags.h"
#include "complex_typedef.h"

/* --- END OF ./headers/variables/variables.h --- */


/* --- START OF ./helpers.c --- */

#include "data.h"

char *cloneString(char *s)
{
    char *copy;
    if (!(*s))
        return "";
    copy = (char *)calloc(strlen(s), sizeof(char *));
    strcpy(copy, s);
    return copy;
}
char *trimFromLeft(char *s)
{
    while (isspace(*s) && *s != '\0')
        s++;
    return s;
}
char *decToHex(int num)
{
    int i = num, size = 0;
    char *hex;
    for (size = 0; i > 0; i = i / 16)
        size++;
    hex = (char *)calloc(size, sizeof(char *));
    sprintf(hex, "%05x", num);
    return hex;
}
char *numToBin(int num)
{
    int i = 0;
    unsigned int result;
    char *word, hex[6];
    word = (char *)calloc(BINARY_WORD_SIZE + 1, sizeof(char *));
    if (num < 0)
    {
        result = abs(num);
        result = ~result;
        result++;
        sprintf(hex, "%05x", (int)(result & 0x4ffff));
    }
    else
        sprintf(hex, "%05x", (int)num & 0xfffff);

    while (hex[i] != '\0')
    {
        switch (hex[i])
        {

        case '0':
            strcat(word, "0000");
            break;
        case '1':
            strcat(word, "0001");
            break;
        case '2':
            strcat(word, "0010");
            break;
        case '3':
            strcat(word, "0011");
            break;
        case '4':
            strcat(word, "0100");
            break;
        case '5':
            strcat(word, "0101");
            break;
        case '6':
            strcat(word, "0110");
            break;
        case '7':
            strcat(word, "0111");
            break;
        case '8':
            strcat(word, "1000");
            break;
        case '9':
            strcat(word, "1001");
            break;
        case 'A':
        case 'a':
            strcat(word, "1010");
            break;
        case 'B':
        case 'b':
            strcat(word, "1011");
            break;
        case 'C':
        case 'c':
            strcat(word, "1100");
            break;
        case 'D':
        case 'd':
            strcat(word, "1101");
            break;
        case 'E':
        case 'e':
            strcat(word, "1110");
            break;
        case 'F':
        case 'f':
            strcat(word, "1111");
            break;
        default:
            break;
        }

        i++;
    }

    strcat(word, "\0");
    return word;
}
HexWord *convertBinaryWordToHex(BinaryWord *word)
{
    int i = 0;
    char hexDigits[4] = {0};
    HexWord *newHex = (HexWord *)malloc(sizeof(HexWord));
    for (i = BINARY_WORD_SIZE - 1; i >= 0; i--)
    {
        hexDigits[i % 4] = word->digit[i].on ? '1' : '0';
        if (i % 4 == 0)
        {
            switch (i)
            {
            case 16:
                newHex->_E = binaryStringToHexNumber(hexDigits);
                break;
            case 12:
                newHex->_D = binaryStringToHexNumber(hexDigits);
                break;
            case 8:
                newHex->_C = binaryStringToHexNumber(hexDigits);
                break;
            case 4:
                newHex->_B = binaryStringToHexNumber(hexDigits);
                break;
            case 0:
                newHex->_A = binaryStringToHexNumber(hexDigits);
                break;
            default:
                break;
            }

            memset(hexDigits, 0, 4);
        }
    }

    return newHex;
}
unsigned binaryStringToHexNumber(char binaryStr[4])
{

    if (!strcmp(binaryStr, "0000"))
        return 0;
    if (!strcmp(binaryStr, "0001"))
        return 1;
    if (!strcmp(binaryStr, "0010"))
        return 2;
    if (!strcmp(binaryStr, "0011"))
        return 3;
    if (!strcmp(binaryStr, "0100"))
        return 4;
    if (!strcmp(binaryStr, "0101"))
        return 5;
    if (!strcmp(binaryStr, "0110"))
        return 6;
    if (!strcmp(binaryStr, "0111"))
        return 7;
    if (!strcmp(binaryStr, "1000"))
        return 8;
    if (!strcmp(binaryStr, "1001"))
        return 9;
    if (!strcmp(binaryStr, "1010"))
        return 0xA;
    if (!strcmp(binaryStr, "1011"))
        return 0xB;
    if (!strcmp(binaryStr, "1100"))
        return 0xC;
    if (!strcmp(binaryStr, "1101"))
        return 0xD;
    if (!strcmp(binaryStr, "1110"))
        return 0xE;
    if (!strcmp(binaryStr, "1111"))
        return 0xF;

    return 0;
}
/* --- END OF ./helpers.c --- */


/* --- START OF ./memory.c --- */

#include "data.h"

static BinaryWord *binaryImg = NULL;
static HexWord *hexImg = NULL;
unsigned static IC = MEMORY_START;
unsigned static DC = 0;
unsigned static ICF = 0;
unsigned static DCF = 0;

extern HexWord *convertBinaryWordToHex(BinaryWord *word);
extern char *numToBin(int num);
extern unsigned binaryStringToHexNumber(char binaryStr[4]);

unsigned getDC() { return DC; }
unsigned getIC() { return IC; }
unsigned getICF() { return ICF; }
unsigned getDCF() { return DCF; }
void increaseDataCounter(int amount)
{
    DC += amount;
}
void increaseInstructionCounter(int amount)
{
    IC += amount;
}
void allocMemoryImg()
{
    const int totalSize = DCF - MEMORY_START;
    int i, j;

    if (binaryImg != NULL)
        free(binaryImg);
    if (hexImg != NULL)
        free(hexImg);

    binaryImg = (BinaryWord *)malloc(totalSize * sizeof(BinaryWord));
    hexImg = (HexWord *)malloc(totalSize * sizeof(HexWord));
    for (i = 0; i < totalSize; i++)
    {
        hexImg[i]._A = 0;
        hexImg[i]._B = 0;
        hexImg[i]._C = 0;
        hexImg[i]._D = 0;
        hexImg[i]._E = 0;

        for (j = 0; j < BINARY_WORD_SIZE; j++)
        {
            binaryImg[i].digit[j].on = 0;
        }
    }
}

void resetMemoryCounters()
{
    IC = MEMORY_START;
    DC = 0;
    ICF = 0;
    DCF = 0;
}
void printBinaryImg()
{
    int i;
    int totalSize = DCF - MEMORY_START;
    for (i = 0; i < totalSize; i++)
    {
        printf("%04d ", MEMORY_START + i);
        printWordBinary(i);
    }
}
void addWord(int value, DataType type)
{
    if (type == Code)
        addWordToCodeImage(numToBin(value));
    else if (type == Data)
        addWordToDataImage(numToBin(value));
}
void addWordToDataImage(char *s)
{
    wordStringToWordObj(s, Data);
    DC++;
}
void addWordToCodeImage(char *s)
{
    wordStringToWordObj(s, Code);
    IC++;
}
void wordStringToWordObj(char *s, DataType type)
{
    int j;
    int index = type == Code ? IC - MEMORY_START : DC - MEMORY_START;
    for (j = 0; j < BINARY_WORD_SIZE; j++)
        binaryImg[index].digit[j].on = s[j] == '1' ? 1 : 0;
}

void printWordBinary(unsigned index)
{
    int j;
    for (j = 0; j < BINARY_WORD_SIZE; j++)
    {
        if (j % 4 == 0)
            printf(" ");
        printf("%d", binaryImg[index].digit[j].on ? 1 : 0);
    }

    printf("\n");
}
void calcFinalAddrsCountersValues()
{

    ICF = IC;
    DCF = ICF + DC;
    DC = IC;
    IC = MEMORY_START;
}

void printMemoryImgInRequiredObjFileFormat()
{
    extern BinaryWord *binaryImg;
    extern HexWord *hexImg;
    int i;
    int totalSize = DCF - MEMORY_START;
    printf("%d %d\n", ICF - MEMORY_START, DCF - ICF);
    for (i = 0; i < totalSize; i++)
    {
        hexImg[i] = *convertBinaryWordToHex(&binaryImg[i]);
        printf("%04d A%x-B%x-C%x-D%x-E%x\n", MEMORY_START + i, hexImg[i]._A, hexImg[i]._B, hexImg[i]._C, hexImg[i]._D, hexImg[i]._E);
    }
}

void writeMemoryImageToObFile(FILE *fp)
{
    extern BinaryWord *binaryImg;
    extern HexWord *hexImg;
    int i;
    int totalSize = DCF - MEMORY_START;
    fprintf(fp, "%d %d\n", ICF - MEMORY_START, DCF - ICF);
    for (i = 0; i < totalSize; i++)
    {
        hexImg[i] = *convertBinaryWordToHex(&binaryImg[i]);
        fprintf(fp, "%04d A%x-B%x-C%x-D%x-E%x\n", MEMORY_START + i, hexImg[i]._A, hexImg[i]._B, hexImg[i]._C, hexImg[i]._D, hexImg[i]._E);
    }
}

/* --- END OF ./memory.c --- */


/* --- START OF ./operations.c --- */

#include "data.h"

static const Operation operations[OP_SIZE] = {
    {0x0001, 0, "mov", {1, 1, 1, 1}, {0, 1, 1, 1}},
    {0x0002, 0, "cmp", {1, 1, 1, 1}, {1, 1, 1, 1}},
    {0x0004, 10, "add", {1, 1, 1, 1}, {0, 1, 1, 1}},
    {0x0004, 11, "sub", {1, 1, 1, 1}, {0, 1, 1, 1}},
    {0x0010, 0, "lea", {0, 1, 1, 0}, {0, 1, 1, 1}},
    {0x0020, 10, "clr", {0, 0, 0, 0}, {0, 1, 1, 1}},
    {0x0020, 11, "not", {0, 0, 0, 0}, {0, 1, 1, 1}},
    {0x0020, 12, "inc", {0, 0, 0, 0}, {0, 1, 1, 1}},
    {0x0020, 13, "dec", {0, 0, 0, 0}, {0, 1, 1, 1}},
    {0x0200, 10, "jmp", {0, 0, 0, 0}, {0, 1, 1, 0}},
    {0x0200, 11, "bne", {0, 0, 0, 0}, {0, 1, 1, 0}},
    {0x0200, 12, "jsr", {0, 0, 0, 0}, {0, 1, 1, 0}},
    {0x1000, 0, "red", {0, 0, 0, 0}, {0, 1, 1, 1}},
    {0x2000, 0, "prn", {0, 0, 0, 0}, {1, 1, 1, 1}},
    {0x4000, 0, "rts", {0, 0, 0, 0}, {0, 0, 0, 0}},
    {0x8000, 0, "stop", {0, 0, 0, 0}, {0, 0, 0, 0}},
};

/* @ Function: Bool isOperationName(char *s)
   @ Arguments: The function gets char *s- the name of the operation it checks if it is valid
   @ Description: The function checks if the operation (char *s) name is valid by using the function
   getOpIndex. Returns true if the operation's name is valid, and false if it isn't.
   */
Bool isOperationName(char *s)
{
    return getOpIndex(s) != -1 ? True : False;
}

/* @ Function: Operation *getOperationByName(char *s)
   @ Arguments: The function gets char *s- the name of the operation that the function returns the information about
   @ Description: The function checks what is the opcode of the operation it got (char *s) using getOpIndex.
   If the operation's opcode is not -1 (which means the operation is valid)
   it returns the information about the operation (from the operations table). Else, it returns NULL.
*/
const Operation *getOperationByName(char *s)
{
    int i = getOpIndex(s);

    if (i != -1)
        return &operations[i];

    return NULL;
}

/* @ Function: Operation *getOperationByIndex(unsigned int i)
   @ Arguments: The function gets unsigned int i which represent the opcode index of the operation
   @ Description: The function checks if i (the opcode index of the operation) is valid- smaller than
   OP_SIZE (which equals to 16- the number of operations).
   If it is valid it returns the information about the operation that the opcode's index i
   represents (from the operations table). Else, it returns NULL.
*/
const Operation *getOperationByIndex(unsigned int i)
{
    return i < OP_SIZE ? &operations[i] : NULL;
}

/* @ Function: char *getOperationName(char *s)
   @ Arguments: The function gets char *s- the name of the operation that the function
   returns its keyword (name)
   @ Description: The function checks if the operation its got (char *s) is a sub string of every
   given operation. If it is, it returns the operation's keyword (name). Else, it returns 0.
*/
const char *getOperationName(char *s)
{
    int i = 0;
    while (i < OP_SIZE)
    {

        if (strstr(operations[i].keyword, s) != NULL)
            return operations[i].keyword;
        i++;
    }
    return 0;
}

/* @ Function: int getOpIndex(char *s)
   @ Arguments: The function gets char *s- the name of the operation it gets it's opcode index
   @ Description: The function compares the operation it got (char *s) with every operation,
   if it is one of the given operations, it returns it's opcode index.
   Else (the operation char *s is not valid), it return -1.
*/
int getOpIndex(char *s)
{
    int i = 0;
    while (i < OP_SIZE)
    {

        if (strcmp(operations[i].keyword, s) == 0)
            return i;
        i++;
    }
    return -1;
}
/* @ Function: Bool isOperationNotStrict(char *s)
   @ Arguments: The function gets char *s- the name of the operation that the function
   checks if it is a non strict given operation.
   @ Description: The function checks if the operation its got (char *s) is a sub string of every
   given operation. If it is, it returns true. Else, it returns false. */
Bool isOperationNotStrict(char *s)
{
    int i = 0;
    while (i < OP_SIZE)
    {

        if (strstr(operations[i].keyword, s) != NULL)
            return True;
        i++;
    }
    return False;
}

/* --- END OF ./operations.c --- */


/* --- START OF ./parse.c --- */

#include "data.h"
static void (*currentLineNumberPlusPlus)() = &increaseCurrentLineNumber;
static void (*resetCurrentLineCounter)() = &resetCurrentLineNumber;
Bool countAndVerifyDataArguments(char *line)
{
    Bool isValid = True;
    int size = 0, n = 0, num = 0;
    char c = 0;
    char args[MAX_LINE_LEN + 1] = {0}, *p;
    line = strstr(line, DATA) + strlen(DATA);

    /* we make the pointer p to point on the position of the first character coming sfter the .data
     instruction within the full line, so that p will point on the begining of the arguments string*/

    /*copies the string of arguments pointer by p into the args local string we will use for parsing*/
    strcpy(args, line);

    isValid = verifyCommaSyntax(args);
    p = strtok(line, ", \t\n\f\r");

    while (p != NULL)
    {
        sscanf(p, "%d%n%c", &num, &n, &c);
        if (c == '.' && n > 0)
            isValid = yieldError(wrongArgumentTypeNotAnInteger);
        num = atoi(p);
        if (!num && *p != '0')
            isValid = yieldError(expectedNumber);

        n = num = c = 0;
        size++;
        p = strtok(NULL, ", \t\n\f\r");
    }

    if (isValid)
        increaseDataCounter(size);

    return isValid;
}

Bool verifyCommaSyntax(char *line)
{
    int commasCounter = 0;
    Bool insideToken = False;
    Bool isFirstToken = True;
    Bool isValid = True;
    char *s = line;
    s = trimFromLeft(s);

    while ((*s == ',' || isspace(*s)) && *s != '\0')
    {
        if (*s == ',')
            commasCounter++;
        s++;
    }
    if (!*s && commasCounter > 0)
        return yieldError(wrongCommasSyntaxIllegalApearenceOfCommasInLine);
    else if (*s && strlen(s) && commasCounter > 0)
        isValid = yieldError(illegalApearenceOfCommaBeforeFirstParameter);

    commasCounter = 0;
    isFirstToken = True;
    while (s && *s != '\0')
    {
        if (insideToken)
        {

            if (commasCounter > 1)
            {
                isValid = yieldError(wrongCommasSyntaxExtra);
                commasCounter = 1;
            }
            else if (commasCounter < 1 && !isFirstToken)
                isValid = yieldError(wrongCommasSyntaxMissing);

            if (isFirstToken == True)
                isFirstToken = False;

            while (*s != '\0' && !isspace(*s) && *s != ',')
                s++;

            if (*s == ',' || isspace(*s))
            {
                insideToken = False;
                commasCounter = 0;
                s--;
            }
        }
        else
        {
            while (*s == ',' || isspace(*s))
            {
                if (*s == ',')
                    commasCounter++;
                s++;
            }

            if (*s && (isprint(*s) && !isspace(*s)))
                insideToken = True;
        }

        s++;
    }

    if (commasCounter)
        isValid = yieldError(illegalApearenceOfCommaAfterLastParameter);

    return isValid;
}

Bool countAndVerifyStringArguments(char *line)
{
    char *args, *closing = 0, *opening = 0;
    int size = 0;
    args = strstr(line, STRING) + strlen(STRING);
    args = trimFromLeft(args);
    if (!*args)
        return yieldError(emptyStringDeclatretion);

    opening = strchr(args, '\"');

    if (!opening || !*opening)
    {
        yieldError(expectedQuotes);
        yieldError(closingQuotesForStringIsMissing);
        return False;
    }
    else
    {
        closing = strrchr(args, '\"');
        if (opening == closing && (opening[0] == args[0]))
            return yieldError(closingQuotesForStringIsMissing);

        if (opening == closing && (opening[0] != args[0]))
            return yieldError(expectedQuotes);
        else
        {
            size = strlen(opening) - strlen(closing);
            increaseDataCounter(size);
        }
    }

    return True;
}

Bool parseLine(char *token, char *line)
{
    State (*globalState)() = &getGlobalState;
    Bool isValid = True;

    if (isComment(token))
        return True;

    if (isLabelDeclaration(token))
    {
        char *next = 0;
        if (!isLabelDeclarationStrict(token))
        {
            char lineClone[MAX_LINE_LEN] = {0}, *rest = 0;
            strcpy(lineClone, line);
            isValid = yieldError(missingSpaceBetweenLabelDeclaretionAndInstruction);
            token = line;
            next = strchr(line, ':');
            next++;
            *next = '\0';
            rest = strchr(lineClone, ':');
            rest++;
            sprintf(line, "%s%c%s", token, ' ', rest);
            strncpy(lineClone, line, strlen(line));
            next = (*globalState)() == firstRun ? strtok(lineClone, " \t\n\f\r") : strtok(lineClone, ", \t\n\f\r");
            return parseLine(next, line) && False;
        }
        else
        {
            next = (*globalState)() == firstRun ? strtok(NULL, " \t\n\f\r") : strtok(NULL, ", \t\n\f\r");
            if (!next)
                return yieldError(emptyLabelDecleration);

            if ((*globalState)() == firstRun)
                return handleLabel(token, next, line) && isValid;
            else
                return isValid && parseLine(next, line + strlen(token) + 1);
        }
    }

    else if (isInstruction(token))
    {
        char *next;
        int type;
        type = getInstructionType(token);
        if (!isInstructionStrict(token))
        {
            isValid = yieldError(missinSpaceAfterInstruction);
            token = getInstructionName(token);
        }
        next = (*globalState)() == firstRun ? strtok(NULL, " \t\n\f\r") : strtok(NULL, ", \t\n\f\r");

        if (isValid && next == NULL)
        {
            if (type == _TYPE_DATA || type == _TYPE_STRING)
                return type == _TYPE_DATA ? yieldWarning(emptyDataDeclaretion) : yieldError(emptyStringDeclatretion);
            else
                return type == _TYPE_ENTRY ? yieldWarning(emptyEntryDeclaretion) : yieldWarning(emptyExternalDeclaretion);
        }
        else if (next != NULL)
        {

            if ((*globalState)() == firstRun)
                return handleInstruction(type, token, next, line) && isValid;
            else
            {
                if (type == _TYPE_DATA)
                    return writeDataInstruction(next) && isValid;
                else if (type == _TYPE_STRING)
                    return writeStringInstruction(next) && isValid;
                else
                    return True;
            }
        }
    }

    else if (isOperation(token))
    {
        char args[MAX_LINE_LEN] = {0};
        strcpy(args, (line + strlen(token)));
        return (*globalState)() == firstRun ? handleOperation(token, args) : writeOperationBinary(token, args);
    }

    else
    {
        if (strlen(token) > 1)
            return yieldError(undefinedTokenNotOperationOrInstructionOrLabel);
        else
            return yieldError(illegalApearenceOfCharacterInTheBegningOfTheLine);
    }

    return isValid;
}

Bool handleSingleLine(char *line)
{
    State (*globalState)() = &getGlobalState;
    char lineCopy[MAX_LINE_LEN] = {0};
    Bool result = True;
    char *token;
    strcpy(lineCopy, line);
    token = ((*globalState)() == firstRun) ? strtok(lineCopy, " \t\n\f\r") : strtok(lineCopy, ", \t\n\f\r");
    result = parseLine(token, line);
    (*currentLineNumberPlusPlus)();
    return result;
}
void parseAssemblyCode(FILE *src)
{
    State (*globalState)() = &getGlobalState;
    void (*setState)(State) = &setGlobalState;
    int c = 0, i = 0;
    char line[MAX_LINE_LEN] = {0};
    Bool isValidCode = True;
    State nextState;
    char *(*fileName)() = &getFileNamePath;

    (*resetCurrentLineCounter)();
    if ((*globalState)() == secondRun)
        printf("\n\n\nSecond Run:(%s)\n", (*fileName)());
    else if ((*globalState)() == firstRun)
        printf("\n\n\nFirst Run:(%s)\n", (*fileName)());

    while (((c = fgetc(src)) != EOF))
    {

        if (isspace(c) && i > 0)
            line[i++] = ' ';

        else if (!isspace(c))
            line[i++] = c;

        if (i >= MAX_LINE_LEN - 2)
            c = '\n';

        if (c == '\n')
        {
            if (i > 0)
            {
                isValidCode = handleSingleLine(line) && isValidCode;
                memset(line, 0, MAX_LINE_LEN);
                i = 0;
            }
        }
    }

    if (i > 0)
        isValidCode = handleSingleLine(line) && isValidCode;

    if (!isValidCode)
        nextState = assemblyCodeFailedToCompile;
    else
        nextState = (*globalState)() == firstRun ? secondRun : exportFiles;

    (*resetCurrentLineCounter)();
    (*setState)(nextState);
}
/* --- END OF ./parse.c --- */


/* --- START OF ./preProccesor.c --- */

#include "data.h"
void (*setState)(State) = &setGlobalState;
State (*globalState)() = &getGlobalState;

void parseMacros(char *line, char *token, FILE *src, FILE *target)
{
    void (*currentLineNumberPlusPlus)() = &increaseCurrentLineNumber;
    static char macroName[MAX_LABEL_LEN] = {0}, *next;
    static Bool isReadingMacro = False;
    static long start = 0, end = 0;
    extern Bool isPossiblyUseOfMacro(char *s);
    extern Bool isMacroOpening(char *s);
    extern Bool isMacroClosing(char *s);
    extern Bool isLegalMacroName(char *s);
    extern Item *addMacro(char *name, int start, int end);
    extern Item *updateMacro(char *name, int start, int end);
    extern Item *getMacro(char *s);

    if (!isReadingMacro)
    {
        if (!isMacroOpening(token))
        {
            fprintf(target, "%s", line);
            (*currentLineNumberPlusPlus)();
        }
    }
    if (!isPossiblyUseOfMacro(token) && !isMacroOpening(token) && !isMacroClosing(token))
        return;

    if (isMacroOpening(token))
    {
        next = strtok(NULL, " \t\n\f\r");

        if (next == NULL)
        {
            yieldError(macroDeclaretionWithoutDefiningMacroName);
            (*setState)(assemblyCodeFailedToCompile);
            return;
        }
        if (!isLegalMacroName(next))
        {
            yieldError(illegalMacroNameUseOfSavedKeywords);
            (*setState)(assemblyCodeFailedToCompile);
            return;
        }
        start = ftell(src);
        strcpy(macroName, next);
        isReadingMacro = True;
    }
    else if (isMacroClosing(token))
    {
        end = ftell(src) - strlen(line) + 1;
        addMacro(macroName, start, end);
        isReadingMacro = False;
        start = end = 0;
        memset(macroName, 0, MAX_LABEL_LEN);
    }
    else if (isPossiblyUseOfMacro(token))
    {
        Item *p = getMacro(token);
        if (p != NULL)
        {
            long c, toCopy = p->val.m.end - p->val.m.start;
            long lastPosition = 0;
            fseek(target, -strlen(line), SEEK_CUR);
            fprintf(target, "%s", "\0");
            lastPosition = ftell(src);
            fseek(src, p->val.m.start, SEEK_SET);

            while (--toCopy && (c = fgetc(src)) != EOF)
                fputc(c, target);

            fseek(src, lastPosition, SEEK_SET);
        }
    }
}

void parseSourceFile(FILE *src, FILE *target)
{
    char line[MAX_LINE_LEN] = {0};
    char lineClone[MAX_LINE_LEN] = {0};
    char *token, c;
    int i = 0;
    void (*resetCurrentLineCounter)() = &resetCurrentLineNumber;
    (*resetCurrentLineCounter)();

    while (((c = fgetc(src)) != EOF))
    {
        line[i++] = c;

        if (i >= MAX_LINE_LEN - 2 && !isspace(c))
            c = '\n';

        if ((*globalState)() == assemblyCodeFailedToCompile)
            return;

        if (c == '\n')
        {
            if (i > 0)
            {
                strncpy(lineClone, line, i);
                token = strtok(lineClone, " \t\n\f\r");
                if (token != NULL)
                    parseMacros(line, token, src, target);

                memset(lineClone, 0, i);
                memset(line, 0, i);
                i = 0;
            }
        }
    }

    if (i > 0)
    {
        strcpy(lineClone, line);
        token = strtok(lineClone, " \t\n\f\r");
        if (token != NULL)
            parseMacros(line, token, src, target);
    }

    if ((*globalState)() != assemblyCodeFailedToCompile)
        (*setState)(firstRun);
}
/* --- END OF ./preProccesor.c --- */


/* --- START OF ./secondRun.c --- */

#include "data.h"

void writeAdditionalOperandsWords(const Operation *op, AddrMethodsOptions active, char *value);
Bool writeOperationBinary(char *operationName, char *args)
{
    const Operation *op = getOperationByName(operationName);
    char *first, *second;
    AddrMethodsOptions active[2] = {{0, 0, 0, 0}, {0, 0, 0, 0}};
    first = strtok(args, ", \t\n\f\r");
    second = strtok(NULL, ", \t\n\f\r");
    writeFirstWord(op);

    if (first && second && (detectOperandType(first, active, 0) && detectOperandType(second, active, 1)))
    {

        writeSecondWord(first, second, active, op);
        writeAdditionalOperandsWords(op, active[0], first);
        writeAdditionalOperandsWords(op, active[1], second);
    }
    else if (!second && first && detectOperandType(first, active, 1))
    {
        second = first;
        writeSecondWord(first, second, active, op);
        writeAdditionalOperandsWords(op, active[1], second);
    }
    else if (!first && !second && !op->funct)
        return True;

    else
        return False;

    return True;
}

void writeAdditionalOperandsWords(const Operation *op, AddrMethodsOptions active, char *value)
{

    if (active.index)
    {
        parseLabelNameFromIndexAddrOperand(value);
        writeDirectOperandWord(value);
    }
    else if (active.direct)
        writeDirectOperandWord(value);
    else if (active.immediate)
        writeImmediateOperandWord(value);
}

Bool writeDataInstruction(char *token)
{
    int num;
    while (token != NULL)
    {
        num = atoi(token);
        addWord((A << 16) | num, Data);
        token = strtok(NULL, ", \t\n\f\r");
    }
    return True;
}

Bool writeStringInstruction(char *s)
{
    char *start = strchr(s, '\"');
    int i, len;
    start++;
    len = strlen(start);
    for (i = 0; i < len - 2; i++)
        addWord((A << 16) | start[i], Data);

    addWord((A << 16) | '\0', Data);
    return True;
}

void writeSecondWord(char *first, char *second, AddrMethodsOptions active[2], const Operation *op)
{
    unsigned secondWord = (A << 16) | (op->funct << 12);
    if (first && (active[0].reg || active[0].index))
        secondWord = secondWord | (active[0].reg ? (getRegisteryNumber(first) << 8) : (parseRegNumberFromIndexAddrOperand(first) << 8)) | (active[0].reg ? (REGISTER_DIRECT_ADDR << 6) : (INDEX_ADDR << 6));
    else if (first && (active[0].direct || active[0].immediate))
        secondWord = secondWord | (0 << 8) | (active[0].direct ? (DIRECT_ADDR << 6) : (IMMEDIATE_ADDR << 6));
    if (second && (active[1].reg || active[1].index))
        secondWord = secondWord | (active[1].reg ? (getRegisteryNumber(second) << 2) : (parseRegNumberFromIndexAddrOperand(second) << 2)) | (active[1].reg ? (REGISTER_DIRECT_ADDR) : (INDEX_ADDR));
    else if (second && (active[1].direct || active[1].immediate))
        secondWord = secondWord | (0 << 2) | (active[1].direct ? (DIRECT_ADDR) : (IMMEDIATE_ADDR));

    addWord(secondWord, Code);
}

void writeFirstWord(const Operation *op)
{
    unsigned firstWord = (A << 16) | op->op;
    addWord(firstWord, Code);
}

void writeDirectOperandWord(char *labelName)
{

    unsigned base = 0, offset = 0;
    if (isExternal(labelName))
    {
        base = getIC();
        addWord((E << 16) | 0, Code);
        offset = getIC();
        addWord((E << 16) | 0, Code);
        updateExtPositionData(labelName, base, offset);
    }

    else
    {
        base = getSymbolBaseAddress(labelName);
        offset = getSymbolOffset(labelName);
        addWord((R << 16) | base, Code);
        addWord((R << 16) | offset, Code);
    }
}

void writeImmediateOperandWord(char *n)
{
    n++;
    addWord((A << 16) | atoi(n), Code);
}

Bool detectOperandType(char *operand, AddrMethodsOptions active[2], int type)
{

    if (isRegistery(operand))
        active[type].reg = 1;
    else if (isValidImmediateParamter(operand))
        active[type].immediate = 1;
    else if (isValidIndexParameter(operand))
        active[type].index = 1;
    else
    {

        if (isSymbolExist(operand))
        {

            if (isEntry(operand) && !isNonEmptyEntry(operand))
                return yieldError(entryDeclaredButNotDefined);

            active[type].direct = 1;
        }
        else
            return yieldError(labelNotExist);
    }
    return True;
}

char *parseLabelNameFromIndexAddrOperand(char *s)
{
    char *p = strchr(s, '[');

    *p = 0;

    return s;
}

int parseRegNumberFromIndexAddrOperand(char *s)
{
    char *p = strchr(s, ']');

    s = strchr(s, '[');
    s++;
    if (p)
        *p = 0;

    return getRegisteryNumber(s);
}

/* --- END OF ./secondRun.c --- */


/* --- START OF ./sharedStates.c --- */

#include "data.h"

static State globalState = startProgram;
static char *path;
static unsigned currentLineNumber = 1;

void setGlobalState(State newState)
{
    globalState = newState;
}
State getGlobalState()
{
    State current = globalState;
    return current;
}

void setFileNamePath(char *s)
{
    if (!*s)
        return;
    path = (char *)realloc(path, strlen(s) * sizeof(char *));
    strcpy(path, cloneString(s));
}

char *getFileNamePath()
{
    return cloneString(path);
}

void resetCurrentLineNumber()
{
    currentLineNumber = 1;
}
void increaseCurrentLineNumber()
{
    currentLineNumber++;
}
int getCurrentLineNumber()
{
    return currentLineNumber;
}

/* --- END OF ./sharedStates.c --- */


/* --- START OF ./tables.c --- */

#include "data.h"
static Item *symbols[HASHSIZE] = {0};
static Item *macros[HASHSIZE] = {0};
static unsigned entriesCount = 0;
static unsigned externalCount = 0;

static ExtListItem *extListHead = NULL;
extern unsigned getICF();
extern Bool verifyLabelNaming(char *s);

void findAllExternals();
void addExtListItem(char *name);
void resetExtList();
ExtListItem *findExtOpListItem(char *name);
void updateExtPositionData(char *name, unsigned base, unsigned offset);
void freeTableItem(Item *item);
void freeTablesMemory();
ExtListItem *findExtOpListItem(char *name)
{
    extern ExtListItem *extListHead;
    ExtListItem *p = extListHead;
    while (p != NULL)
    {
        if (p->name)
        {
            if (strcmp(name, p->name) == 0)
                return p;
        }
        p = p->next;
    }
    return NULL;
}

void resetExtList()
{
    ExtListItem *np = extListHead, *next;
    ExtPositionData *pos, *nextPos;
    externalCount = 0;

    while (np != NULL)
    {
        next = np->next;
        nextPos = np->value.next;
        while (nextPos != NULL)
        {
            pos = nextPos;
            nextPos = pos->next;
            free(pos);
        }
        free(np);
        np = next;
    }

    extListHead = NULL;
}

void updateExtPositionData(char *name, unsigned base, unsigned offset)
{

    ExtListItem *np = findExtOpListItem(name);
    /*     printf("line 62 in table inside update ext position data\nname:%s\nbase:%u\noffset:%u\nnp->name:%s\nnp->value.base:%d\n", name, base, offset, np->name, np->value.base);
     */
    if (np->value.base)
    {
        ExtPositionData *new = (ExtPositionData *)malloc(sizeof(ExtPositionData));
        new->base = base;
        new->offset = offset;
        new->next = np->value.next;
        np->value.next = new;
    }
    else
    {
        np->value.base = base;
        np->value.offset = offset;
    }

    externalCount++;
}

void addExtListItem(char *name)
{

    ExtListItem *next;
    next = (ExtListItem *)calloc(1, sizeof(ExtListItem));
    strncpy(next->name, name, strlen(name));

    if (extListHead != NULL)
    {
        next->next = extListHead->next;
        extListHead->next = next;
    }
    else
    {
        next->next = NULL;
        extListHead = next;
    }
}

unsigned hash(char *s)
{
    unsigned hashval = 1;
    for (hashval = 0; *s != '\0'; s++)
        hashval = *s + 31 * hashval;
    return hashval % HASHSIZE;
}

Item *lookup(char *s, ItemType type)
{
    Item *np;
    int i = hash(s);
    for (np = (type == Symbol ? symbols[i] : macros[i]); np != NULL; np = np->next)
        if (!strcmp(s, np->name))
            return np;

    return NULL;
}

Item *install(char *name, ItemType type)
{
    unsigned hashval;
    Item *np;
    np = (Item *)malloc(sizeof(Item));

    if (np == NULL)
    {
        yieldError(memoryAllocationFailure);
        return NULL;
    }
    else
    {
        np->name = cloneString(name);
        if (type == Symbol)
        {
            np->val.s.attrs.code = 0;
            np->val.s.attrs.entry = 0;
            np->val.s.attrs.external = 0;
            np->val.s.attrs.data = 0;
            np->val.s.base = 0;
            np->val.s.value = 0;
            np->val.s.offset = 0;
        }
        else if (type == Macro)
        {
            np->val.m.start = -1;
            np->val.m.end = -1;
        }

        hashval = hash(name);
        np->next = (type == Symbol ? symbols[hashval] : macros[hashval]);
        if (type == Symbol)
            symbols[hashval] = np;
        else
            macros[hashval] = np;
    }

    return np;
}

Bool addSymbol(char *name, unsigned value, unsigned isCode, unsigned isData, unsigned isEntry, unsigned isExternal)
{
    unsigned base;
    unsigned offset;
    Item *p;

    if (name[strlen(name) - 1] == ':')
        name[strlen(name) - 1] = '\0';

    if (!verifyLabelNamingAndPrintErrors(name))
        return False;
    p = lookup(name, Symbol);
    if (p != NULL)
        return updateSymbol(p, value, isCode, isData, isEntry, isExternal);
    else
    {
        p = install(name, Symbol);
        offset = value % 16;
        base = value - offset;
        p->val.s.value = value;
        p->val.s.base = base;
        p->val.s.offset = offset;
        p->val.s.attrs.code = isCode ? 1 : 0;
        p->val.s.attrs.entry = isEntry ? 1 : 0;
        p->val.s.attrs.external = isExternal ? 1 : 0;
        p->val.s.attrs.data = isData ? 1 : 0;
    }

    return True;
}

Bool updateSymbol(Item *p, unsigned value, unsigned isCode, unsigned isData, unsigned isEntry, unsigned isExternal)
{
    /*     printf("inside updateSymbol\n");
        printf("name:%s value:%d isCode:%u isData:%u isEntry:%u isExternal:%u\n", p->name, value, isCode, isData, isEntry, isExternal);
     */
    /*     printf("inside updateSymbol\n");
     */
    if (((p->val.s.attrs.external) && (value || isData || isEntry || isCode)))
        return yieldError(illegalOverrideOfExternalSymbol);

    else if ((p->val.s.attrs.code || p->val.s.attrs.data || p->val.s.attrs.entry) && isExternal)
        return yieldError(illegalOverrideOfLocalSymbolWithExternalSymbol);

    else
    {
        if ((isData && isCode) || (isCode && p->val.s.attrs.data) || (isData && p->val.s.attrs.code))
            return yieldError(illegalSymbolNameAlreadyInUse);

        if (value)
        {
            unsigned base = 0;
            unsigned offset = 0;
            offset = value % 16;
            base = value - offset;
            p->val.s.value = value;
            p->val.s.base = base;
            p->val.s.offset = offset;
        }

        if (isEntry)
            p->val.s.attrs.entry = 1;
        if (isCode)
            p->val.s.attrs.code = 1;
        if (isData)
            p->val.s.attrs.data = 1;
    }

    return True;
}

Item *getSymbol(char *name)
{
    return lookup(name, Symbol);
}

int getSymbolBaseAddress(char *name)
{
    Item *p = lookup(name, Symbol);
    if (p == NULL)
        return -1;

    return p->val.s.base;
}

int getSymbolOffset(char *name)
{
    Item *p = lookup(name, Symbol);
    if (p == NULL)
        return -1;

    return p->val.s.offset;
}

Bool isSymbolExist(char *name)
{
    return lookup(name, Symbol) != NULL ? True : False;
}
Bool isExternal(char *name)
{
    Item *p = lookup(name, Symbol);
    if (p == NULL)
        return False;
    return p->val.s.attrs.external;
}
Bool isEntry(char *name)
{
    Item *p = lookup(name, Symbol);
    if (p == NULL)
        return False;

    return p->val.s.attrs.entry == 1 ? True : False;
}

Bool isNonEmptyExternal(char *name)
{
    Item *p = lookup(name, Symbol);
    if (p == NULL)
        return False;

    return (p->val.s.attrs.code || p->val.s.attrs.data) ? True : False;
}

Bool isNonEmptyEntry(char *name)
{
    Item *p = lookup(name, Symbol);
    if (p == NULL)
        return False;
    return (p->val.s.attrs.code || p->val.s.attrs.data) ? True : False;
}

Bool isLabelNameAlreadyTaken(char *name, ItemType type)
{
    Item *p = lookup(name, type);

    if (name[strlen(name) - 1] == ':')
        name[strlen(name) - 1] = '\0';

    if (p != NULL)
    {
        if (type == Symbol)
        {
            if (p->val.s.attrs.data || p->val.s.attrs.code)
                return True;
            if (p->val.s.attrs.entry)
                return (!p->val.s.attrs.data && !p->val.s.attrs.code && !p->val.s.attrs.external) ? False : True;
            if (p->val.s.attrs.external)
                return (!p->val.s.attrs.data && !p->val.s.attrs.code && !p->val.s.attrs.entry) ? False : True;
        }

        else if (type == Macro)
            return p->val.m.start != -1 ? False : True;
    }

    return False;
}

Item *updateSymbolAddressValue(char *name, int newValue)
{
    Item *p = getSymbol(name);
    unsigned base;
    unsigned offset;

    if (p != NULL)
    {
        offset = newValue % 16;
        base = newValue - offset;
        p->val.s.offset = offset;
        p->val.s.base = base;
        p->val.s.value = newValue;
    }
    else
        yieldError(symbolDoesNotExist);

    return p;
}

Item *getMacro(char *s)
{
    return lookup(s, Macro);
}

Item *addMacro(char *name, int start, int end)
{
    Item *macro = lookup(name, Macro);

    if (macro != NULL)
    {
        yieldError(illegalMacroNameAlreadyInUse);
        return NULL;
    }
    else
    {
        macro = install(name, Macro);

        if (start != -1)
            macro->val.m.start = start;
        if (end != -1)
            macro->val.m.end = end;
    }

    return macro;
}
Item *updateMacro(char *name, int start, int end)
{
    Item *macro = getMacro(name);
    if (!macro)
        return NULL;
    if (start != -1)
        macro->val.m.start = start;
    if (end != -1)
        macro->val.m.end = end;

    return macro;
}

void updateFinalSymbolTableValues()
{
    int i = 0;
    while (i < HASHSIZE)
    {
        if (symbols[i] != NULL)
            updateFinalValueOfSingleItem(symbols[i]);
        i++;
    }
}

void updateFinalValueOfSingleItem(Item *item)
{
    if (item->val.s.attrs.entry)
        entriesCount++;
    if (item->val.s.attrs.external)
        addExtListItem(item->name);

    if (item->val.s.attrs.data)
    {
        unsigned base = 0, offset = 0, newValue = item->val.s.value + getICF();
        offset = newValue % 16;
        base = newValue - offset;
        item->val.s.offset = offset;
        item->val.s.base = base;
        item->val.s.value = newValue;
    }

    if (item->next != NULL)
        updateFinalValueOfSingleItem(item->next);
}

Bool areEntriesExist()
{
    return entriesCount > 0 ? True : False;
}
Bool areExternalsExist()
{
    return externalCount > 0 ? True : False;
}

void writeExternalsToFile(FILE *fp)
{
    ExtListItem *p = extListHead;
    while (p != NULL)
    {
        if (p->value.base)
            writeSingleExternal(fp, p->name, p->value.base, p->value.offset, p->value.next);
        p = p->next;
    }
}

void writeSingleExternal(FILE *fp, char *name, unsigned base, unsigned offset, ExtPositionData *next)
{

    fprintf(fp, "%s BASE %u\n", name, base);
    fprintf(fp, "%s OFFSET %u\n", name, offset);
    if (next != NULL)
        writeSingleExternal(fp, name, next->base, next->offset, next->next);
}

void writeEntriesToFile(FILE *fp)
{
    int i = 0;
    int totalCount = 0;
    while (i < HASHSIZE)
    {
        if (symbols[i] != NULL && totalCount <= entriesCount)
            totalCount += writeSingleEntry(symbols[i], fp, 0);
        i++;
    }
}

int writeSingleEntry(Item *item, FILE *fp, int count)
{
    if (item->val.s.attrs.entry)
    {
        fprintf(fp, "%s,%d,%d\n", item->name, item->val.s.base, item->val.s.offset);
        count++;
    }
    if (item->next != NULL)
        writeSingleEntry(item->next, fp, count);

    return count;
}

void initTables()
{
    extern unsigned externalCount, entriesCount;
    int i = 0;
    if (extListHead != NULL)
        resetExtList();

    externalCount = entriesCount = 0;
    while (i < HASHSIZE)
    {
        symbols[i] = NULL;
        macros[i] = NULL;
        i++;
    }
}

void freeHashTable(ItemType type)
{

    int i = 0;
    while (i < HASHSIZE)
    {

        if (type == Symbol)
        {
            if (symbols[i] != NULL)
                freeTableItem(symbols[i]);
        }
        else
        {
            if (macros[i] != NULL)
                freeTableItem(macros[i]);
        }

        i++;
    }
}

void freeTableItem(Item *item)
{
    if (item->next != NULL)
        freeTableItem(item->next);
    /*     printf("item->name:%s\n", item->name); */
    free(item);
    return;
}

void printMacroTable()
{
    int i = 0;
    printf("\n\t ~ MACRO TABLE ~ \n");
    printf("\tname\tstart\tend");
    while (i < HASHSIZE)
    {
        if (macros[i] != NULL)
            printMacroItem(macros[i]);
        i++;
    }
    printf("\n\n");
}

int printMacroItem(Item *item)
{

    printf("\n\t%s\t %5d\t%6d", item->name, item->val.m.start, item->val.m.end);
    if (item->next != NULL)
        printMacroItem(item->next);
    return 0;
}

void printSymbolTable()
{
    int i = 0;

    printf("\n\t\t ~ SYMBOL TABLE ~ \n");
    printf("name\tvalue\tbase\toffset\tattributes");

    while (i < HASHSIZE)
    {
        if (symbols[i] != NULL)
            printSymbolItem(symbols[i]);
        i++;
    }
    printf("\n\n");
}

int printSymbolItem(Item *item)
{
    /*  printf("line 94, inside printSymbolItem \n");
     */

    printf("\n%s\t%u\t%u\t%u\t", item->name, item->val.s.value, item->val.s.base, item->val.s.offset);
    if (!item->val.s.attrs.code && !item->val.s.attrs.data && !item->val.s.attrs.entry && !item->val.s.attrs.external)
        printf("   ");

    else
    {
        if ((item->val.s.attrs.code || item->val.s.attrs.data) && (item->val.s.attrs.entry || item->val.s.attrs.external))
        {
            if (item->val.s.attrs.code)
                printf("code,");
            else
                printf("data,");

            if (item->val.s.attrs.entry)
                printf("entry");
            else
                printf("external");
        }
        else
        {
            if (item->val.s.attrs.code)
                printf("code");
            else if (item->val.s.attrs.data)
                printf("data");
            else if (item->val.s.attrs.entry)
                printf("entry");
            else
                printf("external");
        }
    }

    if (item->next != NULL)
        printSymbolItem(item->next);
    return 0;
}
/* --- END OF ./tables.c --- */


/* --- START OF ./utils.c --- */

#include "data.h"

const char *regs[REGS_SIZE] = {"r0", "r1", "r2", "r3", "r4", "r5", "r6", "r7", "r8", "r9", "r10",
                               "r11", "r12", "r13", "r14", "r15"};
Bool isMacroOpening(char *s)
{
    return !strcmp(s, "macro") ? True : False;
}

Bool isMacroClosing(char *s)
{
    return !strcmp(s, "endm") ? True : False;
}
Bool isPossiblyUseOfMacro(char *s)
{
    return !isLabelDeclaration(s) && !isOperation(s) && !isComment(s) && !isInstructionStrict(s) && !isMacroClosing(s) && !isMacroOpening(s);
}

Bool isLegalMacroName(char *s)
{
    return !isInstructionStrict(s) && !isOperation(s) ? True : False;
}

Bool isInstruction(char *s)
{

    if ((!strcmp(s, DATA) || !strcmp(s, STRING) || !strcmp(s, ENTRY) || !strcmp(s, EXTERNAL)))
        return True;

    else if (strstr(s, DATA) != NULL || strstr(s, STRING) != NULL || strstr(s, ENTRY) != NULL || strstr(s, EXTERNAL) != NULL)
    {
        /*         yieldError(missinSpaceAfterInstruction); */
        return True;
    }
    else
        return False;
}

Bool isInstructionStrict(char *s)
{

    return ((!strcmp(s, DATA) || !strcmp(s, STRING) || !strcmp(s, ENTRY) || !strcmp(s, EXTERNAL))) ? True : False;
}

Bool isRegistery(char *s)
{
    int len = strlen(s);
    int i = 0;
    if (s[0] == 'r' && len >= 2)
    {
        while (i < REGS_SIZE)
        {
            if ((strcmp(regs[i], s) == 0))
                return True;
            i++;
        }
    }
    return False;
}
Bool isValidImmediateParamter(char *s)
{
    int i, len = strlen(s);
    if (len < 2 || s[0] != '#' || (!(s[1] == '-' || s[1] == '+' || isdigit(s[1]))))
        return False;
    for (i = 2; i < len; i++)
        if (!isdigit(s[i]))
            return False;
    return True;
}
Bool isIndexParameter(char *s)
{
    int len = strlen(s);
    char *opening = 0, *closing = 0;
    Bool result = True;
    if (len < 5)
        return False;
    else if ((opening = strchr(s, '[')) == NULL || (closing = strchr(s, ']')) == NULL)
        return False;
    else if (closing < opening || (s[len - 1] != ']'))
        return False;
    else
    {
        opening++;
        *closing = '\0';
        if (!isRegistery(opening))
            result = False;
        *closing = ']';
    }
    return result;
}

Bool isValidIndexParameter(char *s)
{
    int len = strlen(s);
    Bool result = True;
    if (len < 6)
        return False;

    else if (!(s[len - 1] == ']' && s[len - 4] == 'r' && s[len - 5] == '['))
        return False;
    else
    {
        char *opening = 0;
        opening = strchr(s, '[');
        opening++;
        s[len - 1] = '\0';

        if (isRegistery(opening) && getRegisteryNumber(opening) < 10)
        {
            result = False;
        }
        s[len - 1] = ']';
    }
    return result;
}

Bool isComment(char *s)
{
    s = trimFromLeft(s);
    return s[0] == ';' ? True : False;
}
Bool isOperation(char *s)
{
    return (getOperationByName(s) != NULL) ? True : False;
}

Bool isLabelDeclarationStrict(char *s)
{
    return s[strlen(s) - 1] == ':' ? True : False;
}

Bool isLabelDeclaration(char *s)
{
    return strchr(s, ':') != NULL ? True : False;
}

int getInstructionType(char *s)
{
    if (strstr(s, DATA) != NULL)
        return _TYPE_DATA;
    if (strstr(s, STRING) != NULL)
        return _TYPE_STRING;
    if (strstr(s, ENTRY) != NULL)
        return _TYPE_ENTRY;
    if (strstr(s, EXTERNAL) != NULL)
        return _TYPE_EXTERNAL;
    return False;
}

int getRegisteryNumber(char *s)
{
    s++;
    return atoi(s);
}

char *getInstructionNameByType(int type)
{
    switch (type)
    {
    case _TYPE_DATA:
        return strcat(DATA, "\0");

    case _TYPE_STRING:
        return strcat(STRING, "\0");

    case _TYPE_ENTRY:
        return strcat(ENTRY, "\0");

    case _TYPE_EXTERNAL:
        return strcat(EXTERNAL, "\0");

    default:
        break;
    }

    return NULL;
}

char *getInstructionName(char *s)
{
    if (strstr(s, DATA) != NULL)
        return DATA;
    if (strstr(s, STRING) != NULL)
        return STRING;
    if (strstr(s, ENTRY) != NULL)
        return ENTRY;
    if (strstr(s, EXTERNAL) != NULL)
        return EXTERNAL;

    return 0;
}

Bool verifyLabelNaming(char *s)
{
    int i = 0;
    int labelLength = strlen(s);

    /* if label name does not start with a alphabet letter */
    if (isalpha(s[0]) == 0)
        return False;

    /* maximum label name length is 31 characters */
    if (labelLength > MAX_LABEL_LEN || labelLength < 1)
        return False;

    if (isRegistery(s))
        return False;

    else if (isOperationName(s))
        return False;
    else
    {

        while (i < labelLength)
        {
            if (!isalnum(s[i]))
                return False;

            i++;
        }
    }

    return True;
}

Bool verifyLabelNamingAndPrintErrors(char *s)
{
    int i = 0;
    int labelLength = strlen(s);

    /* if label name does not start with a alphabet letter */
    if (isalpha(s[0]) == 0)
        return yieldError(illegalLabelNameUseOfCharacters);

    /* maximum label name length is 31 characters */
    else if (labelLength > MAX_LABEL_LEN)
        return yieldError(illegalLabelNameLength);
    else if (labelLength < 1)
        return yieldError(illegalLabelNameLength);

    else if (isRegistery(s))
        return yieldError(illegalLabelNameUseOfSavedKeywordUsingRegisteryName);

    else if (isOperationName(s))
        return yieldError(illegalLabelNameUseOfSavedKeywordUsingOperationName);

    else
    {

        while (i < labelLength)
        {
            if (!isalnum(s[i]))
                return yieldError(illegalLabelNameUseOfCharacters);

            i++;
        }
    }

    return True;
}
/* --- END OF ./utils.c --- */

