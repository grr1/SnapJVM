class Int{
    public static void main(String[] args){
	long i = 3;
	long j = 2;
	if ((i + j) == 5) {
	    System.out.println("3 + 2 = 5");
        }
	if ((i - j) == 1) {
	    System.out.println("3 - 2 = 1");
        }
	long a = 5;
       	long b = 9;
	long c = a * b;
	long d = 3;
	long e = b / d;
	if (c == 45){
		 System.out.println("5 * 9 Equal to 45");
	}
	if (e == d){
		 System.out.println("9 / 3 Equal to 3");
	}
	long f = 4;
	if ((a | f) == a) {
	    System.out.println("4 | 5 equal to 5");
        }
	if ((a & f) == f) {
	    System.out.println("4 & 5 equal to 4");
        }
	long g = 12;
	if ((a ^ b) == g) {
	    System.out.println("5 ^ 9 equal to 12");
        }
	else {
		System.out.println(a^b);
	}

    }
}
