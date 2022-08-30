package hw4;

import api.Expression;
import api.Instruction;
import api.ArgList;
import api.Function;
import api.Scope;
import api.ParameterList;

/**
 * Expression representing the value of a function call. All expressions
 * in the argument list are evaluated with respect to the current scope,
 * and the resulting values are associated with the corresponding parameter names in
 * a new "local" Scope.  This local scope is used to evaluate the function body
 * (its BlockInstruction) and after that, the function return expression. 
 * Variables in the current scope are not accessible during execution of the function
 * body.
 * The eval method of this call expression returns value of the function's
 * return expression.
 * <ul>
 *   <li>There are two children; the first is the Function object, and the second 
 *   is the argument list.
 *   <li>The getLabel() method returns the string "Call".
 *   <li>The getText() method returns the getText() string of the Function
 * </ul>
 */
// TODO: THIS CLASS MUST DIRECTLY OR INDIRECTLY IMPLEMENT THE Expression INTERFACE
// AND OVERRIDE THE toString METHOD
public class CallExpression extends SuperAbstract implements Expression
{
	/**
	 * Function to represent a function method
	 */
	private Function function;
	
	/**
	 * ArgList which contains Expressions to represent a functions arguments
	 */
	private ArgList arguments;
	
  /**
   * Constructs a CallExpression for the given function and argument list.
   * @param f
   *   the function to be called
   * @param args
   *   the arguments to the function
   */
  public CallExpression(Function f, ArgList args)
  {
	  super("Call");
	  addNode(f);
	  addNode(args);
	  function = f;
	  arguments = args;
  }
  
  /**
   * Sets the Function object for this CallExpression
   * @param f
   *   the function to be called
   */
  public void setFunction(Function f)
  {
	  function = f;
  }
  
  @Override
  public int eval(Scope env)
  {
	  //new local scope
	  Scope local = new Scope();
	  for (int i = 0; i < function.getChild(0).getNumChildren(); i++)
	  {
		  String paramName = ((Identifier)((ParameterList)function.getChild(0)).getChild(i)).getText();
		  //evaluate args with "current" scope
		  int argEval = ((Expression)arguments.getChild(i)).eval(env);
		  //put variables into local scope
		  local.put(paramName, argEval);
	  }
	  //evaluate body and returnExpression
	  Instruction body = (Instruction)function.getChild(1);
	  body.execute(local);
	  Expression returnExpression = (Expression)function.getChild(2);
	  return returnExpression.eval(local);
  }
  
  @Override
  public String getText()
  {
    return function.getText();
  }

}
