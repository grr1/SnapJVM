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

	//we cannot push a negative number yet
	
	int negative = -6;
	switch(negative){
	case -7:
	    System.out.println("-7");
	    break;
	case -6:
	    System.out.println("-6");
	    break;
	case -4:
	    System.out.println("-4");
	    break;
	case -3:
	case -2:
	case -1:
	    System.out.println("-1<=x<=-3");
	    break;
	case 0:
	    System.out.println("0");
	    break;
	case 1:
	    System.out.println("1");
	    break;
        default:
	    System.out.println("not in range of [-7, 1]");
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
