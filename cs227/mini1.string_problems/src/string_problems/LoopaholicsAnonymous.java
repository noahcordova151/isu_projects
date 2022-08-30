package string_problems;
import java.util.Scanner;

/**
 * 
 * @author Noah Cordova
 * Utility class with some loop practice problems.
 */
public class LoopaholicsAnonymous
{
	
	/**
	 * Private constructor prevents instantiation.
	 */
	private LoopaholicsAnonymous() {}
	
	/**
	 * Determines how many iterations of the following operation   
	 * are required until the condition (a * a + b * b) &gt; 4 is reached.
	 * If the condition (a * a + b * b) &gt; 4
	 * is not reached within <code>maxIterations</code>, 
	 * the method returns <code>maxIterations</code>.
	 * @param x 
	 * 	given x value
	 * @param y 
	 * 	given y value 
	 * @param 
	 * 	maxIterations
	 * 	maximum number of iterations to attempt   
	 * @return   
	 * 	number of iterations required to get (a * a + b * b) &gt; 4,
	 * 	or maxIterations
	 */
	public static int findEscapeCount(double x, double y, int maxIterations)  
	{
		double a = 0;
		double b = 0;
		double newA;
		double newB;
		int i = 0;
		
		while((a * a + b * b <= 4) && (i < maxIterations))
		{
			i++;
			newA = a * a - b * b + x;
			newB = 2 * a * b + y;
			a = newA;
			b = newB;
		}
		
		return i;
	}
	
	/**
	 * Returns the index for the nth occurrence of the given character
	 * in the given string, or -1 if the character does not occur n or more times.
	 * This method is case sensitive.
	 * @param s
	 * 	given string
	 * @param ch
	 * 	given character to find
	 * @param n
	 * 	occurrence to find
	 * @return
	 * 	index of nth occurrence, or -1 if the character does not occur
	 * 	n or more times
	 */
	public static int findNth(String s, char ch, int n)
	{
		int count = 0;
		for (int i = 0; i < s.length(); i++)
		{
			if (s.charAt(i) == ch)
			{
				count++;
			}
			
			if (count == n)
			{
				return i;
			}
		}
		return -1;
	}
	
	/**
	 * Returns the longest substring starting at the given position
	 * in which all characters are the same.
	 * Returns an empty string if the given start index is out of range.
	 * @param s
	 * 	given string
	 * @param start
	 * 	index at which to start the run
	 * @return
	 * 	substring of all matching characters starting at the given position
	 */
	public static String getRun(String s, int start)
	{
		if (start < 0 || start > s.length() - 1) {
			return "";
		}
		
		int counter = 1;
		for (int i = start; (i < s.length() - 1) && (s.charAt(i) == s.charAt(i+1)); i++)
		{
			counter++;
		}
		
		return s.substring(start, start + counter);
	}
	
	/**
	 * Given a string of text containing numbers separated by
	 * commas, returns true if the numbers form an <em>arithmetic</em> sequence
	 * (a sequence in which each value differs from the previous one by a fixed amount).
	 * The method should return true for any string containing
	 * two or fewer numbers and false for any invalid string.
	 * @param text
	 * 	a string of text containing numbers separated by commas
	 * @return
	 * 	true if each number differs from the previous one by the same amount
	 */
	public static boolean isArithmetic(String text)
	{
		if (text.length() == 0)
		{
			return true;
		}
		
		//First try with scanner
		Scanner scan = new Scanner(text);
		scan.useDelimiter(",");

		int next = 0;
		int dif = 0;
		
		//checks if there are 3 integers, setting dif of first two if possible
		if (scan.hasNextInt()) 
		{
			int first = scan.nextInt();
			if (scan.hasNextInt())
			{
				next = scan.nextInt();
				dif = next - first;
				if (!scan.hasNextInt() && !scan.hasNext())
				{
					scan.close();
					return true;
				}
			}
			else
			{
				scan.close();
				return true;
			}
		}
		else
		{
			scan.close();
			return false;
		}
		
		while (scan.hasNextInt()) {
			int nextNext = scan.nextInt();
			
			if((nextNext - next) != dif) 
			{
				scan.close();
				return false;
			}
			
			next = nextNext;
		}
		
		//only works for invalid at end
		if (scan.hasNext())
		{
			scan.close();
			return false;
		}
		scan.close();
		return true;
		
	}
	
	/**
	 * Determines whether the string <code>target</code> occurs as a subsequence
	 * of string <code>source</code> where "gaps" are allowed between characters
	 * of <code>target</code>. That is, the characters in <code>target</code>
	 * occur in <code>source</code> in their given order but do not have to be
	 * adjacent.  (Pictured another way, this method returns true if
	 * <code>target</code> could be obtained from <code>source</code>
	 * by removing some of the letters of <code>source</code>.)
	 * This method is case sensitive. 
	 * @param source
	 * 	the given string in which to find the target characters
	 * @param target
	 * 	the characters to be found
	 * @return
	 * 	true if the characters in <code>target</code> can be found
	 * 	as a subsequence in <code>source</code>, false otherwise
	 */
	public static boolean subsequenceWithGaps(String source, String target)
	{
		String newSource = "";

		for (int i = 0; i < source.length(); i++)
		{
			if (target.contains(String.valueOf(source.charAt(i))))
			{
				newSource += String.valueOf(source.charAt(i));
			}
			
			if (newSource.contains(target)) {
				return true;
			}
		}
		
		return false;
	}
	
	/**
	 * Separates s into two strings, each made of alternating characters
	 * from s, except that runs of the same character are kept together.
	 * The two strings are concatenated with a space between them to make
	 * a single returned string. If the given string is empty, the returned
	 * string is a single space.
	 * @param s
	 * 	any string
	 * @return
	 * 	pair of strings obtained by taking alternating characters from s, 
	 * 	keeping runs of the same character together, concatenated with
	 * 	one space between them into a single string
	 */
	public static String takeApartPreservingRuns(String s)
	{
		String s1 = "";
		String s2 = "";
		
		int j;
		int iterationCounter = 1;
		for (int i = 0; i < s.length() - 1; i += (j - i + 1))
		{
			//find run
			j = i;
			while ((j < s.length() - 1) && (s.charAt(j) == s.charAt(j + 1)))
			{
				j++;
			}
			
			//add to s1 or s2
			if (iterationCounter % 2 == 1)
			{
				s1 += s.substring(i, j + 1);
				if (i == s.length() - 2)
				{
					s2 += s.charAt(i + 1);
				}
			}
			else
			{
				s2 += s.substring(i, j + 1);
				if (i == s.length() - 2)
				{
					s1 += s.charAt(i + 1);
				}
			}
			
			iterationCounter++;
		}
		
		if (s.length() == 1)
		{
			s1 += s.charAt(0);
		}
		
		return s1 + " " + s2;
	}
	
	/**
	 * Returns the longest substring of consecutive characters in a given
	 * string that are in ascending (nondecreasing) order.
	 * If there are multiple runs of the same maximal length,
	 * the methods returns the leftmost one.
	 * @param s
	 * 	any given string
	 * @return
	 * 	longest nondecreasing substring
	 */
	public static String longestAscendingSubstring(String s)
	{
		String maxRun = "";
		
		int j;
		for (int i = 0; i < s.length() - 1; i += (j - i + 1))
		{
			//find run
			j = i;
			while ((j < s.length() - 1) && (s.charAt(j) <= s.charAt(j + 1)))
			{
				j++;
			}
			
			//save run
			String thisRun = s.substring(i, j + 1);
			if (thisRun.length() > maxRun.length())
			{
				maxRun = thisRun;
			}
		}
		
		return maxRun;
	}
	
	/**
	 * Prints a pattern of 2n rows containing dashes and stars,
	 * as illustrated below for n = 5.  Note that the first row
	 * starts with exactly n - 1 spaces and the middle rows have
	 * no spaces.
	 * @param n 
	 * 	one-half the number of rows in the output
	 */
	public static void printStars(int n)
	{
		//top half
		for (int row = 0; row < n; row++)
		{
			for (int i = 0; i < n - (row + 1); i++)
			{
				System.out.print("-");
			}
			System.out.print("*");
			for (int i = 0; i < (2 * row); i++)
			{
				System.out.print(" ");
			}
			System.out.print("*");
			for (int i = 0; i < n - (row + 1); i++)
			{
				System.out.print("-");
			}
			System.out.println("");
		}
		
		//bottom half
		for (int row = n - 1; row >= 0; row--)
		{
			for (int i = 0; i < n - (row + 1); i++)
			{
				System.out.print("-");
			}
			System.out.print("*");
			for (int i = 0; i < (2 * row); i++)
			{
				System.out.print(" ");
			}
			System.out.print("*");
			for (int i = 0; i < n - (row + 1); i++)
			{
				System.out.print("-");
			}
			System.out.println("");
		}
	}
	
}