package hw4;

import api.Expression;
import api.Scope;

/**
 * Arithmetic negation expression (unary minus).  
 * <ul>
 *   <li>There is one child, the expression to be negated
 *   <li>The getLabel() method returns the string "Negative".
 *   <li>The getText() method returns an empty string.
 * </ul>
 */
// TODO: THIS CLASS MUST DIRECTLY OR INDIRECTLY IMPLEMENT THE Expression INTERFACE
// AND OVERRIDE THE toString METHOD
public class AopNegation extends UnaryExpressionAbstract
{
  /**
   * Constructs a new unary expression representing the negative
   * of the given expression.
   * @param expr
   *   arithmetic expression to be negated
   */
  public AopNegation(Expression expr)
  {
	  super(expr, "Negative");
  }
  
  @Override
  public int eval(Scope env)
  {
    int leftVal = getLhs().eval(env);
    return leftVal * -1;
  }

}
