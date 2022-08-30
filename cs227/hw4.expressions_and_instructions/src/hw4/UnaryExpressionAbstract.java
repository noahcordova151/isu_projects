package hw4;

import api.Expression;

public abstract class UnaryExpressionAbstract extends SuperAbstract implements Expression
{
	/**
	 * Expression to represent unary child Expression
	 */
	  private Expression lhs;
	  
	  /**
	   * Constructs an Expression with given Expression and label
	   * @param lhs
	   *    Given Expression
	   * @param givenLabel
	   * 	Given String to represent label
	   */
	  protected UnaryExpressionAbstract(Expression lhs, String givenLabel)
	  {
		super(givenLabel);
		addNode(lhs);
	    this.lhs = lhs;
	  }
	  
	  /**
	   * Returns the Expression instance variable
	   * @return lhs
	   * 		Expression instance variable
	   */
	  public Expression getLhs()
	  {
		  return lhs;
	  }
}