<?php
/*
 * phc -- the open source PHP compiler
 * See doc/license/README.license for licensing information
 *
 * Test an unparsed script interprets the same as the original after running
 * the shredder, using php command line.
 */
array_push($tests, new InterpretShredded ());
class InterpretShredded extends TwoCommandTest
{
	function get_test_subjects ()
	{
		return get_interpretable_scripts ();
	}

	function allow_failure ()
	{
		return true;
	}

	function get_command_line1 ($subject)
	{
		global $phc, $php;
		$dir_name = dirname($subject);
		return "cat $subject | $php -d include_path=$dir_name:./ 2>&1";
	}

	function get_command_line2 ($subject)
	{
		global $phc, $php;
		$dir_name = dirname($subject);
		return "$phc --run-shredder --run goto_uppering.la --dump-php $subject 2>&1 | $php -d include_path=$dir_name:./ 2>&1";
	}

	function homogenize_output ($string)
	{
		return $this->homogenize_line_numbers ($string);
	}

}
?>