#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <termios.h>
#include <unistd.h>



#define MAX_LINE_LENGTH 10000
#define SIZE 10000000
#define CONDITIONS 3



char getch() {
    struct termios oldt, newt;
    char ch;

    // Get the current terminal attributes
    tcgetattr(STDIN_FILENO, &oldt);
    newt = oldt;

    // Disable canonical mode and echo
    newt.c_lflag &= ~(ICANON | ECHO);
    tcsetattr(STDIN_FILENO, TCSANOW, &newt);

    // Read a single character
    ch = getchar();

    // Restore the old terminal attributes
    tcsetattr(STDIN_FILENO, TCSANOW, &oldt);

    return ch;
}


int getString(char *str){
    int i = 0;
    while((str[i]=getch())!='\n'){
        if(str[i]=='\e') return 0;//esc pressed
        printf("%c",str[i]==0x7F?'\b':str[i]);//because replit detects backspace as delete for some reason
        if(str[i] == 0x08 || str[i]== 0x7F){
            if(i>0){
                printf(" \b");
                i--;
            }
        }
        else i++;
    }
    str[i]='\0';
    printf("\n");
    return 1;
}
void replace_special_chars(char *str) {
    int len = strlen(str);
    int i = 0;

    while (i < len) {
        if (strncmp(str + i, "&amp;", 5) == 0) {
            strncpy(str + i, "&", 1);
            memmove(str + i + 1, str + i + 5, len - i - 4);
            len -= 4;
        } else if (str[i] == '\\') {
            memmove(str + i, str + i + 1, len - i);
            len--;
        } else {
            i++;
        }
    }
}

char *find(char *str, char *res, short int cond, char *key) {
    char *pos, *pos1, *pos2;

    if (cond == 1 && (pos = strstr(str, "FBQualityClass")) != NULL) {
        char q[5];
        snprintf(q, 5, "%4s", pos + strlen("FBQualityClass=\\\"sd\\\" FBQualityLabel=\\") + 1);
        pos1 = strstr(pos, "u003CBaseURL>") + strlen("u003CBaseURL>");
        pos2 = strstr(pos1, "\\u003C");
        if (pos2 == NULL) return NULL;
        (*pos2) = '\0';

        replace_special_chars(pos1);
        snprintf(res, MAX_LINE_LENGTH, "\033[1mQuality:%s\033[0m\n\033[1;36m\033[4m%s\033[0m", q, pos1);
        return pos2;
    } else if (cond == 2 && (pos = strstr(str, "data-plyr-embed-id=")) != NULL) {
        int yt = 0;
        pos1 = pos + strlen("data-plyr-embed-id=\"");
        pos2 = strstr(pos1, "\"");

        if (strstr(str, "youtube") != NULL) yt = 1;

        str[pos2 - str] = '\0';
        if (yt)
            snprintf(res, MAX_LINE_LENGTH, "\033[1;36m\033[4mhttps://www.youtube.com/watch?v=%s\033[0m", pos1);
        else
            strncpy(res, pos1, MAX_LINE_LENGTH);
        return pos2;
    } 
    else if (cond == 3 && (pos2 = strstr(str, ".mp4")) != NULL) {
        char *temp;
        
        pos1=str;
        while(pos1 && pos1<=pos2){
            temp=pos1;
            pos1=strstr(pos1+1,"http");
        }
        pos1=temp;
        char *pos2_1,*pos2_2;
        pos2_1=strstr(pos2, "\\u003C");
        pos2_2=strstr(pos2, "\"");
        if(pos2_1==NULL) pos2=pos2_2;
        if(pos2_2==NULL && pos2_1==NULL) return NULL;
        if(pos2_2==NULL) pos2=pos2_1;
        if(pos2_1 && pos2_2) pos2=(pos2_1-pos2)<(pos2_2-pos2)?pos2_1:pos2_2;
        (*pos2) = '\0';
        
        snprintf(res, MAX_LINE_LENGTH, "\033[1;36m\033[4m%s\033[0m", pos1);
        replace_special_chars(res);
        return pos2;
    } 
    else if (cond == 4 && (pos2 = strstr(str, key)) != NULL) {
        char *temp;

        pos1=str;
        while(pos1 && pos1<=pos2){
            temp=pos1;
            pos1=strstr(pos1+1,"http");
        }
        pos1=temp;
        char *pos2_1,*pos2_2;
        pos2_1=strstr(pos2, "\\u003C");
        pos2_2=strstr(pos2, "\"");
        if(pos2_1==NULL) pos2=pos2_2;
        if(pos2_2==NULL && pos2_1==NULL) return NULL;
        if(pos2_2==NULL) pos2=pos2_1;
        if(pos2_1 && pos2_2) pos2=(pos2_1-pos2)<(pos2_2-pos2)?pos2_1:pos2_2;
        (*pos2) = '\0';

        snprintf(res, MAX_LINE_LENGTH, "\033[1;36m\033[4m%s\033[0m", pos1);
        replace_special_chars(res);
        return pos2;
    } 
    return NULL;
}

int main() {
    char res[100][MAX_LINE_LENGTH + 2];
    
    char *s = (char *)malloc(SIZE + 2);

    FILE *file = fopen("input.txt", "r");

    if (file == NULL) {
        fprintf(stderr, "Unable to open input.txt.\n");
        free(s);
        return 1;
    }

    size_t read_size = fread(s, sizeof(char), SIZE, file);
    if (!read_size) {
        printf("File is empty. Enter text in input.txt. Then run the program again.\n");
        return 1;
    }
    s[read_size] = '\0';
    fclose(file);

    char *constS = (char *)malloc(SIZE + 2);
    strcpy(constS, s);
    char *p;
    char *initialS = s;

    char cond[][MAX_LINE_LENGTH] = {
        "FB vid",
        "Embedded YT vid",
        "mp4 file",
        "default search"
    };

    int i = 0;
    for (int I = 1; I <= CONDITIONS; I++) {
        int found = 0, newline = 1;
        int next = 0;

        for (; (*s) != '\0' && (p = find(s, res[i], I, "")) != NULL;) {
            for (int j = 0; j < i; j++) {
                if (i != 0 && strcmp(strstr(res[i], "http"), strstr(res[j], "http")) == 0) {
                    next = 1;
                    break;
                }
            }
            if (next) {
                next = 0;
                continue;
            }
            found = 1;
            if (newline) {
                puts("\n");
                newline = 0;
            }
            printf("\033[1;32m\033[1m");   // Set text color to bright green and bold
            puts(cond[I - 1]);     
            printf("\033[0m");     // Reset text color to default

            printf("URL %d\n%s\n", i + 1, res[i]);
            i++;
            s = p + 1;
        }
        s = initialS;
        strcpy(s, constS);
    }

    printf("Do you want to search with keyword?(y/n): ");
    char ch;
    while ((ch = getch()) == '\n' || ch == ' ');
    printf("%c",ch);
    if (ch == 'Y' || ch == 'y') {
        strcpy(s, constS);
        
        
        while (1) {
            int found = 0, newline = 1;
            
            int next = 0;
            int i = 0;
            printf("\nEnter keyword, or press esc to exit:");
            char tempo;
            
            
            
            if(!getString(cond[3]) ) {
            
            puts("\033[1;31m\nesc detected. Exiting program...\033[0m");
            
            break;
            }
            
            
            for (; (*s) != '\0' && (p = find(s, res[i], 4, cond[3])) != NULL;) {
                for (int j = 0; j < i; j++) {
                    if (i != 0 && strcmp(strstr(res[i], "http"), strstr(res[j], "http")) == 0) {
                        next = 1;
                        break;
                    }
                }
                if (next) {
                    next = 0;
                    continue;
                }
                found = 1;
                if (newline) {
                    puts("\n");
                    newline = 0;
                }

                
                printf("URL %d\n%s\n", i + 1, res[i]);
                i++;
                s = p + 1;
            }
    
            
            s = initialS;
            strcpy(s, constS);
            
        }
        
    }

    FILE *file1 = fopen("input.txt", "w");  // To clear input.txt

    free(s);
    free(constS);
    return 0;
}
