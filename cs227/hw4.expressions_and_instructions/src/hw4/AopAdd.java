package hw4;

import api.Expression;
import api.Scope;

/**
 * Node type representing an arithmetic expression 
 * with an addition operator.
 * <ul>
 *   <li>There are two children; the first is the left operand, and the second 
 *   is the right operand.
 *   <li>The getLabel() method returns the string "+".
 *   <li>The getText() method returns an empty string.
 * </ul>
 */
//TODO: POSSIBLY REFACTOR THIS CLASS TO REDUCE CODE DUPLICATION
public class AopAdd extends BinaryExpressionAbstract
{
 
  /**
   * Constructs an expression with the given left and right sides.
   * @param lhs
   *   expression for the left-hand-side operand
   * @param rhs
   *   expression for the left-hand-side operand
   */
  public AopAdd(Expression lhs, Expression rhs)
  {
	  super(lhs,rhs, "+");
  }
  
  @Override
  public int eval(Scope env)
  {
    int leftVal = getLhs().eval(env);
    int rightVal = getRhs().eval(env);
    return leftVal + rightVal;
  }
}
