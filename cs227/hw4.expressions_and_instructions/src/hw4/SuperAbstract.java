package hw4;

import parser.ProgramNode;
import java.util.ArrayList;
import api.DefaultNode;

public abstract class SuperAbstract implements ProgramNode
{
	/**
	 * String to represent the label for each class
	 */
	private String label;
	
	/**
	 * ArrayList for instance variable ProgramNodes used by each class
	 */
	private ArrayList<ProgramNode> nodes;
	
	/**
	 * Constructs a class with a label and ArrayList for instance variable ProgramNodes 
	 * @param label
	 * 		Given String to represent label
	 */
	protected SuperAbstract(String label)
	{
		this.label = label;
		nodes = new ArrayList<ProgramNode>();
	}
	
	@Override
	public ProgramNode getChild(int i)
	{
		if (i < nodes.size())
		{
			return nodes.get(i);
		}
		else
		{
			return new DefaultNode("Invalid index " + i + " for type " + this.getClass().getName());
		}
	}
	
	@Override
	public int getNumChildren()
	{
		return nodes.size();
	}
	
	/**
	 * Method to be used by subclasses to add nodes to ProgramNode ArrayList
	 * @param node
	 * 		ProgramNode passed in by subclass
	 */
	protected void addNode(ProgramNode node)
	{
		nodes.add(node);
	}
	
	@Override
	public String getLabel()
	{
		return label;
	}
	
	@Override
	  public String getText()
	  {
	    return "";
	  }
	
	@Override
	public String toString()
	{
	  return makeString();
	}
}