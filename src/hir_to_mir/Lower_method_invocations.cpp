/*
 * phc -- the open source PHP compiler
 * See doc/license/README.license for licensing information
 *
 * Lower the parameters of method invocations, adding a run-time check to
 * determine if the parameter is passed by reference.
 */

#include "Lower_method_invocations.h"

using namespace HIR;

/* Convert
 *
 *		$t1->x ($$var1, $t1->$var2[$i]);
 *
 *	into
 *
 *		// fetch param 0
 *		if (param_is_ref ($t1, x, 0))
 *			$T1 =& $$var1;
 *		else
 *			$T1 = $$var1;
 *
 *		// fetch param 1
 *		if (param_is_ref ($t1, x, 1))
 *		{
 *			$T2 =& $t1->$$var2;
 *			$T3 =& $T2[$i];
 *		}
 *		else
 *		{
 *			$T2 = $t1->$$var2;
 *			$T3 = $T2[$i];
 *		}
 *
 *		// Call the function
 *		$t1->x ($T1, $T3);
 */
Expr*
Lower_method_invocations::pre_method_invocation (Method_invocation* in)
{
	// Leave unset in place - it still needs access to the target and indices.
	METHOD_NAME* mn = dynamic_cast <METHOD_NAME*> (in->method_name);
	if (in->target == NULL
		&& mn
		&& (*mn->value == "unset"
			|| *mn->value == "isset"))
		return in;

	int index = 0;
	foreach (Actual_parameter* actual_param, *in->actual_parameters)
	{
		Variable_actual_parameter* ap = dyc<Variable_actual_parameter> (actual_param);

		// Ignore simple variables
		if (ap->target == NULL
			&& isa<VARIABLE_NAME> (ap->variable_name)
			&& ap->array_indices->size () == 0)
			continue;

		// TODO: check the is_ref flag. Make sure we don't lose this information.
		// and transform differently is this case.
		FOREIGN* param_is_ref = new FOREIGN (
			new MIR::Param_is_ref (
				in->target ? folder.fold_target (in->target): NULL,
				folder.fold_method_name (in->method_name),
				new MIR::PARAM_INDEX (index)));

		List<Statement*>* iftrue = new List<Statement*>;
		List<Statement*>* iffalse = new List<Statement*>;

		pieces->push_back (
			new If (
				eval (check_builtin (param_is_ref)),
				iftrue,
				iffalse));


		// Shred the parameter. This is a specialized version of
		// AST_shredder::post_variable.

		VARIABLE_NAME* prev = NULL;

		// Shred variable_variables with no target.
		if (ap->target == NULL 
				&& isa<Variable_variable> (ap->variable_name))
		{
			VARIABLE_NAME* temp = fresh_var_name ("TMIr");

			iftrue->push_back(
					new Assign_var(
						temp->clone (),
						true,
						ap->variable_name));

			prev = temp;
		}

		// Shred the target
		if(ap->target)
		{
			VARIABLE_NAME* temp = fresh_var_name ("TMIt");
			iftrue->push_back (
				new Assign_var (
							temp->clone (),
							true,
							new Target_expr (
								ap->target,
								ap->variable_name->clone ())));
			prev = temp;
			ap->target = NULL;
		}

		if (prev == NULL)
			prev = dyc<VARIABLE_NAME> (ap->variable_name);

		// Shred the array indices
		while (ap->array_indices->size ())
		{
			VARIABLE_NAME* temp = fresh_var_name ("TMIi");
			iftrue->push_back(
				new Assign_var (
					temp->clone (),
					true,
					new Index_array (
						prev->clone (),
						ap->array_indices->front ())));

			prev = temp;
			ap->array_indices->pop_front();
		}

		assert (ap->target == NULL);
		assert (ap->array_indices->size() == 0);
		ap->variable_name = prev;

		// Copy the iftrue to iffalse, except assignments arent by reference.
		foreach (Statement* s, *iftrue)
		{
			Assign_var* assign = (dyc<Assign_var> (s))->clone ();
			assign->is_ref = false;
			iffalse->push_back (assign);
		}
	}

	return in;
}

/* Builtins don't have symbol table entries to be looked up at run-time.
 * However, we know the answer in all these cases. */
Expr*
Lower_method_invocations::check_builtin (FOREIGN* in)
{
	MIR::Param_is_ref* param_is_ref = dyc<MIR::Param_is_ref> (in->value);

	if (param_is_ref->target == NULL
		&& isa<MIR::METHOD_NAME> (param_is_ref->method_name))
	{
		String* method_name = dyc<MIR::METHOD_NAME> (param_is_ref->method_name)->value;
		if (*method_name == "isset")
			return new BOOL (false);
	}

	return in;
}