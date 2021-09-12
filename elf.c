unsigned int main(int a, int b) {
    return a + b; 
}

int getMe(int x){
    return x + 5;
}

int (*foo) (int);

unsigned int test(int a, int b) {
    
    foo = (void*)b ;
    b = foo(b);
    return a + b; 
}


//void main(void) {
//    sum(a, b)
//};
