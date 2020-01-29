class SimpleArray{
    static int [] fieldArray;
    public static void main(String args[]) {
	fieldArray = new int[4];
	int [] localArrayX = new int[5];
	int [] localArrayY = {1, 3, 5, 7};
	int xLength = localArrayX.length;
	System.out.println(xLength);
	int yLength = localArrayY.length;
	System.out.println(yLength);
	localArrayX[0] = 2;
	localArrayY[1] = 4;
	int sum = localArrayX[0] + localArrayY[3];
	System.out.println(sum);
        
    }

}
