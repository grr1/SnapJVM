class Switch{
    public static void main(String[] args){
	//tableswitch
	int dayOfWeek = 4;
	switch(dayOfWeek){
	case 1:
	case 2:
	case 3:
	case 4:
	case 5:
	    System.out.println("Weekday");
	    break;
	case 6:
	case 7:
	    System.out.println("Weekend");
	    break;
	default:
	    System.out.println("Invalid");
	}
	
	
	//lookupswitch
	int number = 12;
	switch(number){
	case 1:
	    System.out.println("1");
	    break;
	case 12:
	    System.out.println("12");
	    break;
	case 123:
	    System.out.println("123");
	    break;
	case 1234:
	    System.out.println("1234");
	    break;
	default:
	    System.out.println("I don't know");
	}
	
	System.out.println("Switch Completed");
    }
}
