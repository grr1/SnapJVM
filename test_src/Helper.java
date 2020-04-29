class Helper {
    public static void main(String args[]) {
        int i = 5;
        i = helper(i);
        System.out.println("After helper\n");
        //System.out.println("i="+i);
    }

    public static int helper(int x) {
        System.out.println("Inside helper\n");
        return x*3;
    }
    /*public static int helper() {
        return 15;
    }*/
}
