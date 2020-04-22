class Long{
    public static void main(String[] args){
	long i = 1356099454469L;
	long j = 2L;
	if ((i + j) == 1356099454471L) {
	    System.out.println("1356099454469L + 2 = 1356099454471L");
        }
	if ((i - j) == 1356099454467L) {
	    System.out.println("1356099454469L - 2 = 1356099454467L");
        }
	long a = 5L;
       	long b = 9L;
	long c = a * b;
	long d = 3L;
	long e = b / d;
	if (c == 45L){
		 System.out.println("5 * 9 Equal to 45");
	}
	if (e == d){
		 System.out.println("9 / 3 Equal to 3");
	}
	long f = 4L;
	if ((a | f) == a) {
	    System.out.println("4 | 5 equal to 5");
        }
	if ((a & f) == f) {
	    System.out.println("4 & 5 equal to 4");
        }
	long g = 12L;
	if ((a ^ b) == g) {
	    System.out.println("5 ^ 9 equal to 12");
        }
	else {
		System.out.println(a^b);
	}

    }
}
