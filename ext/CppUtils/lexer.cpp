#include <stdio.h>
#include <time.h>

#include "vector.h"
#include "macros.h"

typedef struct{
	const char* start;
	int length;
} Token;

int TokenEqual(Token a, Token b){
	if(a.length == b.length){
		return memcmp(a.start, b.start, a.length) == 0;
	}
	
	return 0;
}

// TODO: Move this into strings.h
size_t FindChar(const char* str, char c){
	const char* cursor = str;
	while(*cursor){
		
		if(*cursor == c){
			return (cursor - str);
		}
		
		cursor++;
	}
	
	return -1;
}

typedef enum{
	STRING,
	CHARACTER,
	INTEGER,
	FLOAT,
	OCTAL,
	HEX,
	OPERATOR,
	IDENTIFIER,
	LINECOMMENT,
	ANNOTATION_STR,
	ANNOTATION_ID,
	ANNOTATION_WS,
	BLOCKCOMMENT,
	WHITESPACE
} ParseState;

#define COMPARE_STRING(ptr, reference) (memcmp(ptr, reference, (sizeof(reference) - 1)) == 0)
#define COMPARE_STRING_N(ptr, reference, len) (memcmp(ptr, reference, BNS_MIN(sizeof(reference) - 1, len)) == 0)

#define TOKEN_IS(token, str) (COMPARE_STRING_N(token.start, str, token.length) && token.length == (sizeof(str)-1))

#define MAKE_TOKEN(str) {str, sizeof(str)-1}

static const char* whitespace = "\t\r\n ";

Vector<Token> LexString(const char* string){
	static const char* operators[] = {",", "++", "--", "*", "/", "->", "+", "-", "&", "|", "&&", "||", "#", "%", "{", "}", ";", "(", ")"
									 "~", "^", "!=", "==", "=", "!", ".", "?", ":", "<", ">", "<<", ">>", "<=", ">=", "[", "]", "+=",
									 "-=", "*=", "/=", "^=", "|=", "&=", "##"};
	
	static const Token annoStart = {"/*[", 3};
	static const Token annoEnd   = {"]*/", 3};
	
	Vector<Token> tokens;	
	ParseState currState = WHITESPACE;
	
	#define EMIT_TOKEN() {currToken.length = fileCursor - currToken.start+1;tokens.PushBack(currToken);\
						  currToken.length = 0; currToken.start = fileCursor+1;}
	
	int fileSize = strlen(string);
	
	const char* fileCursor = string;
	Token currToken = {fileCursor, 0};
	
	while(fileCursor - string < fileSize){
		switch(currState){
			case STRING:{
				if(*fileCursor == '\\'){
					fileCursor++;
				}
				else if(*fileCursor == '"'){
					EMIT_TOKEN();
					currState = WHITESPACE;
				}
			}break;
			
			case CHARACTER:{
				if(*fileCursor == '\\'){
					fileCursor++;
				}
				else if(*fileCursor == '\''){
					EMIT_TOKEN();
					currState = WHITESPACE;
				}
			}break;
			
			case INTEGER:{
				if(*fileCursor == '.'){
					currState = FLOAT;
				}
				else if(*fileCursor < '0' || *fileCursor > '9'){
					fileCursor--;
					EMIT_TOKEN();
					currState = WHITESPACE;
				}
			}break;
			
			case FLOAT:{
				if(*fileCursor < '0' || *fileCursor > '9'){
					fileCursor--;
					EMIT_TOKEN();
					currState = WHITESPACE;
				}
			}break;
			
			case OCTAL:{
				if(*fileCursor < '0' || *fileCursor > '9'){
					fileCursor--;
					EMIT_TOKEN();
					currState = WHITESPACE;
				}
				else if(*fileCursor == 'x' || *fileCursor == 'X'){
					currState = HEX;
				}
			}break;
			
			case HEX:{
				if((*fileCursor < '0' || *fileCursor > '9') && (*fileCursor < 'a' || *fileCursor > 'f')
					&& (*fileCursor < 'A' || *fileCursor > 'F')){
					fileCursor--;
					EMIT_TOKEN();
					currState = WHITESPACE;
				}
			}break;
			
			case OPERATOR:{
				bool found = false;

				for(int i = 0; i < BNS_ARRAY_COUNT(operators); i++){
					if(memcmp(currToken.start, operators[i], fileCursor - currToken.start + 1) == 0){
						found = true;
						break;
					}
				}
				
				if(!found){
					fileCursor--;
					EMIT_TOKEN();
					currState = WHITESPACE;
				}
			}break;
			
			case IDENTIFIER:{		
				if(FindChar(whitespace, *fileCursor) != -1){
					fileCursor--;
					EMIT_TOKEN();
					currState = WHITESPACE;
				}
				else{
					bool isOp = false;
					for(int  i = 0; i < BNS_ARRAY_COUNT(operators); i++){
						if(operators[i][0] == *fileCursor){
							isOp = true;
							break;
						}
					}
					
					if(isOp){
						fileCursor--;
						EMIT_TOKEN();
						currState = WHITESPACE;
					}
				}
			}break;
			
			case LINECOMMENT:{
				if(*fileCursor == '\n'){
					currState = WHITESPACE;
				}
				
				currToken.start++;
			}break;
			
			case ANNOTATION_STR:{
				if(*fileCursor == '"'){
					EMIT_TOKEN();
					break;
				}
			}
			case ANNOTATION_ID:{
				if(*fileCursor == ']'){
					fileCursor--;
					EMIT_TOKEN();
					tokens.PushBack(annoEnd);
					currState = BLOCKCOMMENT;
				}
				else if(FindChar(whitespace, *fileCursor) != -1 || *fileCursor == '('){
					fileCursor--;
					EMIT_TOKEN();
					currState = ANNOTATION_WS;
				}
			}break;

			case ANNOTATION_WS:{
				if(*fileCursor == ']'){
					tokens.PushBack(annoEnd);
					currState = BLOCKCOMMENT;
					break;
				}
				else if(*fileCursor == '(' || *fileCursor == ')'){
					Token thisTok = {fileCursor, 1};
					tokens.PushBack(thisTok);
					break;
				}
				else if(*fileCursor == '"'){
					currState = ANNOTATION_STR;
				}
				else if(FindChar(whitespace, *fileCursor) == -1){
					currState = ANNOTATION_ID;
				}
			}
			
			case BLOCKCOMMENT:{
				if(fileCursor - string < fileSize - 2 && fileCursor[0] == '*' && fileCursor[1] == '/'){
					currState = WHITESPACE;
					fileCursor += 2;
					currToken.start += 2;
				}
				else if(*fileCursor == '[' && *(fileCursor - 1) == '*' && *(fileCursor - 2) == '/'){
					tokens.PushBack(annoStart);
					currState = ANNOTATION_WS;
				}
				
				currToken.start++;
			}break;
			
			case WHITESPACE:{
				currToken.start = fileCursor;
				if(fileCursor - string < fileSize - 2 && fileCursor[0] == '/' && fileCursor[1] == '/'){
					currState = LINECOMMENT;
				}
				else if(fileCursor - string < fileSize - 2 && fileCursor[0] == '/' && fileCursor[1] == '*'){
					currState = BLOCKCOMMENT;
				}
				else if(FindChar(whitespace, *fileCursor) == -1){
					
					if(*fileCursor == '"'){
						currState = STRING;
						fileCursor++;
					}
					else if(*fileCursor == '\''){
						currState = CHARACTER;
						fileCursor++;
					}
					else if(*fileCursor == '0'){
						currState = OCTAL;
					}
					else if(*fileCursor >= '1' && *fileCursor <= '9'){
						currState = INTEGER;
					}
					else{
						bool isOp = false;
						for(int i = 0; i < BNS_ARRAY_COUNT(operators); i++){
							if(operators[i][0] == *fileCursor){
								isOp = true;
								break;
							}
						}
						
						if(isOp){
							currState = OPERATOR;
						}
						else{
							currState = IDENTIFIER;
						}
					}
					
					fileCursor--;
				}
			}break;
		}
		
		fileCursor++;
	}
	
	if(currState != WHITESPACE){
		fileCursor--;
		EMIT_TOKEN();
		fileCursor++;
	}
	
	#undef EMIT_TOKEN
	
	return tokens;
}

#if defined(LEXER_TEST_MAIN)

int main(int argc, char** argv){
	
	BNS_UNUSED(argc);
	BNS_UNUSED(argv);
	
	{
		ASSERT(FindChar("abbbbc", 'c') == 5);
		ASSERT(FindChar("abbbbc", 'a') == 0);
		ASSERT(FindChar("abbbbc", 'd') == -1);
		ASSERT(FindChar("abbbbc", 'A') == -1);
		ASSERT(FindChar("  \t", '\t') == 2);
	}
	
	{
		Vector<Token> lexedToks = LexString("1+2*3");
		
		ASSERT(lexedToks.count == 5);
		ASSERT(TOKEN_IS(lexedToks.data[0], "1"));
		ASSERT(TOKEN_IS(lexedToks.data[1], "+"));
		ASSERT(TOKEN_IS(lexedToks.data[2], "2"));
	}
	
	{
		Vector<Token> lexedToks = LexString("if (x == 2)  \t\t\t\n\n\n\n334");
		
		ASSERT(lexedToks.count == 7);
		
		ASSERT(TOKEN_IS(lexedToks.data[6], "334"));
	}
	
	{
		Token tok = MAKE_TOKEN("55543");
		ASSERT(TOKEN_IS(tok, "55543"));
		
		const char* string = "if(this.Marvel() == that->goal()){iter++;}";
		const Token expectedToks[] = {MAKE_TOKEN("if"), MAKE_TOKEN("("), MAKE_TOKEN("this"), MAKE_TOKEN("."),
									  MAKE_TOKEN("Marvel"), MAKE_TOKEN("("), MAKE_TOKEN(")"), MAKE_TOKEN("=="), 
									  MAKE_TOKEN("that"), MAKE_TOKEN("->"), MAKE_TOKEN("goal"), MAKE_TOKEN("("), 
									  MAKE_TOKEN(")"), MAKE_TOKEN(")"), MAKE_TOKEN("{"), MAKE_TOKEN("iter"), 
									  MAKE_TOKEN("++"), MAKE_TOKEN(";"), MAKE_TOKEN("}")}; 
								   
		Vector<Token> actualToks = LexString(string);
		
		ASSERT(BNS_ARRAY_COUNT(expectedToks) == actualToks.count);
		
		for(int i = 0; i < actualToks.count; i++){
			ASSERT(TokenEqual(actualToks.data[i], expectedToks[i]));
		}
	}

#if 0
	{
		FILE* thisFile = fopen("lexer.cpp", "rb");
		
		fseek(thisFile, 0, SEEK_END);
		int fileLength = ftell(thisFile);
		fseek(thisFile, 0, SEEK_SET);
		
		char* fileContents = (char*)malloc(fileLength + 1);
		fread(fileContents, 1, fileLength, thisFile);
		fileContents[fileLength] = '\0';
		fclose(thisFile);
		
		Vector<Token> lexedFile = LexString(fileContents);
		
		for(int i = 0; i < lexedFile.count; i++){
			printf("|%.*s|\n", lexedFile.data[i].length, lexedFile.data[i].start);
		}
		
		free(fileContents);
	}
#endif
	
	return 0;
}

#endif
