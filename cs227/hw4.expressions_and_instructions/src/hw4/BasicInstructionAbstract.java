package hw4;

import api.Instruction;
import api.Expression;

public abstract class BasicInstructionAbstract extends SuperAbstract implements Instruction
{
	/**
	 * Expression used by every Instruction subclass
	 */
	private Expression expression;
	
	/**
	 * Constructs an Instruction with given Expression and label
	 * @param expr
	 * 		Given Expression
	 * @param givenLabel
	 * 		Given String to represent label
	 */
	protected BasicInstructionAbstract(Expression expr, String givenLabel)
	{
		super(givenLabel);
		addNode(expr);
		expression = expr;
	}
	
	/**
	 * Constructs an Instruction with given Identifier, Expression, and label
	 * @param v
	 * 		Given Identifier
	 * @param expr
	 * 		Given Expression
	 * @param givenLabel
	 * 		Given label
	 */
	protected BasicInstructionAbstract(Identifier v, Expression expr, String givenLabel)
	{
		super(givenLabel);
		addNode(v);
		addNode(expr);
		expression = expr;
	}
	
	/**
	 * Returns Expression instance variable
	 * @return
	 * 		Expression instance variable 
	 */
	public Expression getExpression()
	{
		return expression;
	}
}
