package hw4;

import api.DefaultNode;
import api.Instruction;
import api.Scope;
import parser.ProgramNode;

import java.util.ArrayList;

/**
 * Compound instruction representing a sequence of instructions.  Execution
 * of a block iterates through the sequence, calling execute() on each
 * one. When initially constructed, the block contains no instructions.  
 * Instruction objects are added to the sequence using the method
 * <pre>
 *     public void addStatement(Instruction s)
 * </pre>
 * <ul>
 *   <li>The number of children is the number of statements in
 *       this block (possibly zero).
 *   <li>The getLabel() method returns the string "Block".
 *   <li>The getText() method returns an empty string
 * </ul>
 */
//TODO: THIS CLASS MUST DIRECTLY OR INDIRECTLY IMPLEMENT THE Instruction INTERFACE
// AND OVERRIDE THE toString METHOD
public class BlockInstruction extends SuperAbstract implements Instruction
{
	/**
	 * ArrayList of Instructions to execute
	 */
	private ArrayList<Instruction> instructions;

  /**
   * Constructs an empty sequence of instructions.
   */
  public BlockInstruction()
  {
	  super("Block");
	  instructions = new ArrayList<Instruction>();
  }

  /**
   * Adds an instruction to this block.  The instructions will be executed
   * in the order added.
   * @param s
   *   instruction to be added to this block
   */
  public void addStatement(Instruction s)
  {
	  instructions.add(s);
  }

  @Override
  public ProgramNode getChild(int i)
  {
    if (i < instructions.size())
    {
    	return instructions.get(i);
    }
    else
    {
      return new DefaultNode("Invalid index " + i + " for type " + this.getClass().getName());      
    }
  }

  @Override
  public int getNumChildren()
  {
    return instructions.size();
  }

  @Override
  public void execute(Scope env)
  {
	  for (Instruction s : instructions)
	  {
		  s.execute(env);
	  }
  }

}
