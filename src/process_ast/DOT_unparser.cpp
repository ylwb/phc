/*
 * phc -- the open source PHP compiler
 * See doc/license/README.license for licensing information
 *
 * Convert the phc AST to DOT format
 */

#include <iostream>
#include "lib/demangle.h"
#include "DOT_unparser.h"
#include "cmdline.h"

using namespace std;

extern struct gengetopt_args_info args_info;

void DOT_unparser::show_source_rep(String* source_rep)
{
	if(source_rep)
		cout << "node_" << new_node_id << " [label=\"" << *source_rep << "\", shape=box]" << endl;
	else
		cout << "node_" << new_node_id << " [label=\"???\", shape=box]" << endl;
	node_stack.push(new_node_id++);
}

DOT_unparser::DOT_unparser() 
{
	new_node_id = 0;
}

DOT_unparser::~DOT_unparser() 
{
}

void DOT_unparser::pre_node(AST_node* in)
{
	if(dynamic_cast<AST_php_script*>(in))
	{
		cout << "/*" << endl;
		cout << " * AST in dot format generated by phc -- the PHP compiler" << endl;
		cout << " */" << endl << endl;
		cout << "digraph AST {" << endl;
	}
	
	cout << "node_" << new_node_id << " [label=\"" << demangle(in);
	if(!args_info.no_line_numbers_flag && in->get_line_number() != 0)
		cout << " (" << in->get_line_number() << ")";
	cout << "\"];" << endl;	

	node_ids[in] = new_node_id;
	node_stack.push(new_node_id++);
}

void DOT_unparser::post_node(AST_node* in)
{
	int src_id = node_ids[in];
	while(node_stack.top() != src_id)
	{
		int target_id = node_stack.top();	
		cout << "node_" << src_id << " -> node_" << target_id << ";" << endl;
		node_stack.pop();
	}

	if(dynamic_cast<AST_php_script*>(in))
		cout << "}" << endl;
}

void DOT_unparser::pre_identifier(AST_identifier* in)
{
	show_source_rep(in->get_value_as_string());
}

void DOT_unparser::pre_literal(AST_literal* in)
{
	String& str = *in->get_source_rep();
	ostringstream escaped;
	unsigned i;

	for(i = 0; i < 10 && i < str.size(); i++)
	{
		switch(str[i])
		{
			case '\\':
			case '"':
				escaped << "\\" << str[i];
				break;
			default:
				if(str[i] >= 32 && str[i] != 127) escaped << str[i];
				break;
		}
	}	

	if(i < str.size())
		escaped << "..";

	show_source_rep(new String(escaped.str()));
}

void DOT_unparser::visit_null(char* type_id)
{
	cout << "node_" << new_node_id << " [label=\"NULL\\n(" << type_id << ")\", shape=plaintext]" << endl;
	node_stack.push(new_node_id++);
}

void DOT_unparser::visit_marker(char* name, bool value)
{
	if(value)
	{
		cout << "node_" << new_node_id << " [label=\"" << name << "\", shape=plaintext]" << endl;
		node_stack.push(new_node_id++);
	}
}