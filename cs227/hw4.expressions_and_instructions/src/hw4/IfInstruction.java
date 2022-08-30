package hw4;

import api.Expression;
import api.Instruction;
import api.Scope;

/**
 * Instruction type representing a conditional.  A conditional has an
 * expression and two instructions.  If the expression evaluates to a 
 * nonzero value (i.e., "true"), the first instruction is executed; otherwise, the 
 * second instruction is executed.
 * <ul>
 *   <li>There are three children; the first is the expression, the second 
 *   is the instruction for the "true", the third is the instruction for
 *   the "false" branch
 *   <li>The getLabel() method returns the string "If".
 *   <li>The getText() method returns an empty string.
 * </ul>
 */
//TODO: THIS CLASS MUST DIRECTLY OR INDIRECTLY IMPLEMENT THE Instruction INTERFACE
// AND OVERRIDE THE toString METHOD
public class IfInstruction extends BasicInstructionAbstract
{
	/**
	 * Instruction to execute if true
	 */
	private Instruction s0;
	
	/**
	 * Instruction to execute if false
	 */
	private Instruction s1;
  
  /**
   * Constructs a conditional statement from the given condition
   * and alternative actions.
   * @param condition
   *    an expression to use as the condition
   * @param s0
   *    statement to be executed if the condition is true (nonzero)
   * @param s1
   *    statement to be executed if the condition is false (zero)
   */
  public IfInstruction(Expression condition, Instruction s0, Instruction s1)
  {
	  super(condition,"If");
	  addNode(s0);
	  addNode(s1);
	  this.s0 = s0;
	  this.s1 = s1;
  }

  @Override
  public void execute(Scope env)
  {
    int value = getExpression().eval(env);
    if (value != 0)
    {
    	s0.execute(env);
    }
    else
    {
    	s1.execute(env);
    }
  }

}
