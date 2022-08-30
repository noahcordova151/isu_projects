package hw4;

import api.Expression;

public abstract class BinaryExpressionAbstract extends UnaryExpressionAbstract
{
	/**
	 * Expression to represent right side of an expression
	 */
	  private Expression rhs;
	  
	  /**
	   * Constructs an Expression with given left and right sides
	   * @param lhs
	   *   given Expression to represent left side
	   * @param rhs
	   *   given Expression to represent right side
	   * @param givenLabel
	   * 	Given String to represent label
	   */
	  protected BinaryExpressionAbstract(Expression lhs, Expression rhs, String givenLabel)
	  {		
		super(lhs, givenLabel);
		addNode(rhs);
	    this.rhs = rhs;
	  }
	  
	  /**
	   * Returns the Expression instance variable for right side
	   * @return rhs
	   * 		Expression instance variable for right side
	   */
	  public Expression getRhs()
	  {
		  return rhs;
	  }
}
