package hw4;

import api.Expression;
import api.Scope;

/**
 * Node type representing a logical expression 
 * with the "or" operator.  If both operands evaluate
 * to zero, then this expression evaluates to 0;
 * otherwise, this expression evaluates to 1.
 * <ul>
 *   <li>There are two children; the first is the left operand, and the second 
 *   is the right operand.
 *   <li>The getLabel() method returns the string "||".
 *   <li>The getText() method returns an empty string.
 * </ul>
 */
//TODO: THIS CLASS MUST DIRECTLY OR INDIRECTLY IMPLEMENT THE Expression INTERFACE
// AND OVERRIDE THE toString METHOD
public class LopOr extends BinaryExpressionAbstract
{
  /**
   * Constructs an expression with the given left and right sides.
   * @param lhs
   *   expression for the left-hand-side operand
   * @param rhs
   *   expression for the left-hand-side operand
   */
  public LopOr(Expression lhs, Expression rhs)
  {
	  super(lhs,rhs, "||");
  }
  
  @Override
  public int eval(Scope env)
  {
    int leftVal = getLhs().eval(env);
    int rightVal = getRhs().eval(env);
    if(leftVal != 0 || rightVal != 0)
    {
    	return 1;
    }
    return 0;
  }

}