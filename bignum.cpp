#include <stdlib.h>
#include <stdio.h>
#include <iostream>

class BigNum{
    public:
        int length;

        char num[];
        
        BigNum(char * number, int len);

        void print_num(){
            for (int i = 0; i < length; i++){
                printf("%d", num[i]);
            }
            printf("\n");
        }
};

BigNum::BigNum(char* number, int len){
    for (int i = 0; i < len; i++){
        num[i] = *number;
        number = number + 1;
    }
    length = len;
}

int main(){

    char test_num[10];
    std::cout << "Enter a number here: " << std::endl;
    std::cin >> test_num;

    BigNum test = BigNum(test_num, 9);
    std::cout << "You entered: " << std::endl;
    test.print_num();

    return 0;

}