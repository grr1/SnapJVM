class Loop{
    public static void main(String args[]) {
	
	int a = 0;
	int step = 2;
	int b = 9;
	while(a < b){
	    System.out.println("While Loop: Looping...");
	    a += step;
	}
	System.out.println("While Loop: Loop Ends");
        
	for(a = 0; a < b; a += step){
	    System.out.println("For Loop: Looping...");
	}
	System.out.println("For Loop: Loop Ends");
    }

}
