class SimpleArray{
    //static int [] fieldArray;
    public static void main(String args[]) {
	//fieldArray = new int[4];
	int [] intArray = new int[5];
	intArray[1] = 7;
	intArray[2] = 4;
	if(intArray[1] == 7 && intArray[2] == 4 && intArray[3] == 0){
	    System.out.println("Correct Elements in Int Array!");
	}
	if(intArray.length == 5){
	    System.out.println("Correct Int Array Length!");
	}
	
	char [] charArray = {'a', 'd', 'k', 'x'};
	charArray[2] = 'p';
	if(charArray[0] == 'a' && charArray[2] == 'p'){
	    System.out.println("Correct Elements in Char Array!");
	}
	if(charArray.length == 4){
	    System.out.println("Correct Char Array Length!");
	}


	boolean [] booleanArray = {true, false, true};
	if(booleanArray[0] && !booleanArray[1]){
	    System.out.println("Correct Element in Boolean Array!");
	}
	
	/*
	
	short [] localArrayY = {1, 2, 5, 7};
	if(localArrayY[0] + localArrayY[3] == 8){
	    System.out.println("Correct Elements in Short Array!");
	}
	if(localArrayY.length == 4){
	    System.out.println("Correct Short Array Length!");
	}

	*/
	

	/*
	
	int xLength = localArrayX.length;
	System.out.println(xLength);
	int yLength = localArrayY.length;
	System.out.println(yLength);
	localArrayX[0] = 2;
	localArrayY[1] = 4;
	int sum = localArrayX[0] + localArrayY[3];
	System.out.println(sum);
        */
    }

}
