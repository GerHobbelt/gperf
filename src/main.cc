/* Driver program for the hash function generator
   Copyright (C) 1989-1998, 2000, 2002-2003, 2009, 2017, 2025 Free Software Foundation, Inc.
   Written by Douglas C. Schmidt <schmidt@ics.uci.edu>
   and Bruno Haible <bruno@clisp.org>.

   This file is part of GNU GPERF.

   This program is free software: you can redistribute it and/or modify
   it under the terms of the GNU General Public License as published by
   the Free Software Foundation; either version 3 of the License, or
   (at your option) any later version.

   This program is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY; without even the implied warranty of
   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
   GNU General Public License for more details.

   You should have received a copy of the GNU General Public License
   along with this program.  If not, see <https://www.gnu.org/licenses/>.  */

#include <config.h>

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "options.h"
#include "input.h"
#include "search.h"
#include "output.h"
#include "output-javascript.h"
#include "output-lua.h"

#ifdef PERF
#include "../tests/perf.h"
#endif

/* ------------------------------------------------------------------------- */

/* This Keyword factory produces KeywordExt instances.  */

typedef Keyword_Factory<KeywordExt> KeywordExt_Factory;

template <>
KeywordExt *
Keyword_Factory<KeywordExt>::create_keyword (const char *allchars, int allchars_length,
                                             const char *rest, unsigned int lineno)
{
  return new KeywordExt (allchars, allchars_length, rest, lineno);
}

// constructor:
template <>
Keyword_Factory<KeywordExt>::Keyword_Factory()
{
}

// constructor:
template <>
Keyword_Factory<KeywordExt>::~Keyword_Factory()
{
}

/* ------------------------------------------------------------------------- */

// see bottom of keyword.cc for more info why this template specialization statement has been moved here...

/* Explicit template instantiations.  Needed to avoid link-time errors.

   C++ is just misdesigned:  The most important aspect in building large
   software packages is information hiding.  (That's the point of having the
   implementation of a .h file in a .cc file, isn't it?  And of having
   classes with private fields and methods, isn't it?)  The fact that we
   need the instantiation of the Keyword_Factory<KT> class only for
   KT = KeywordExt comes from the code in main.cc.  It is ugly that
   implementation details of main.cc have an influence into this file here.  */

template class Keyword_Factory<KeywordExt>;

// --> and moving this here from keyword.cc resulted in new warnings, which were kinda expected
// as the constructor and destructor don't have default implementations specified in the template
// definition:
//
// - warning C4661: 'Keyword_Factory<KeywordExt>::Keyword_Factory(void)': no suitable definition provided for explicit template instantiation request
// - warning C4661: 'Keyword_Factory<KeywordExt>::~Keyword_Factory(void)': no suitable definition provided for explicit template instantiation request
//
// --> provide these as well, *above*!

/* ------------------------------------------------------------------------- */

int
main (int argc, char *argv[])
{
  int exitcode;

  /* Set the Options.  Open the input file and assign stdin to it.  */
  option.parse_options (argc, argv);

  /* Open the input file.  */
  if (option.get_input_file_name ())
    if (!freopen (option.get_input_file_name (), "r", stdin))
      {
        fprintf (stderr, "Cannot open input file '%s'\n",
                 option.get_input_file_name ());
        return 1;
      }

#ifdef PERF
  uint64_t t0 = timer_start();
#endif
  {
    /* Initialize the keyword list.  */
    KeywordExt_Factory factory;
    Input<KeywordExt> inputter (stdin, &factory);
    inputter.read_input ();
    /* We can cast the keyword list to KeywordExt_List* because its list
       elements were created by KeywordExt_Factory.  */
    KeywordExt_List* list = inputter._head;

    {
      /* Search for a good hash function.  */
      Search searcher (list);
      /* Output the hash function code.  */
	  Output *outputter;
        if (option[JAVASCRIPT])
          {
            outputter = new OutputJavascript(searcher._head,
                                        inputter._struct_decl,
                                        inputter._struct_decl_lineno,
                                        inputter._return_type,
                                        inputter._struct_tag,
                                        inputter._verbatim_declarations,
                                        inputter._verbatim_declarations_end,
                                        inputter._verbatim_declarations_lineno,
                                        inputter._verbatim_code,
                                        inputter._verbatim_code_end,
                                        inputter._verbatim_code_lineno,
                                        inputter._charset_dependent,
                                        searcher._total_keys,
                                        searcher._max_key_len,
                                        searcher._min_key_len,
                                        searcher._hash_includes_len,
                                        searcher._key_positions,
                                        searcher._alpha_inc,
                                        searcher._total_duplicates,
                                        searcher._alpha_size,
                                        searcher._asso_values);
          }
        else if (option[LUA])
          {
			outputter = new OutputLua(searcher._head,
                                 inputter._struct_decl,
                                 inputter._struct_decl_lineno,
                                 inputter._return_type,
                                 inputter._struct_tag,
                                 inputter._verbatim_declarations,
                                 inputter._verbatim_declarations_end,
                                 inputter._verbatim_declarations_lineno,
                                 inputter._verbatim_code,
                                 inputter._verbatim_code_end,
                                 inputter._verbatim_code_lineno,
                                 inputter._charset_dependent,
                                 searcher._total_keys,
                                 searcher._max_key_len,
                                 searcher._min_key_len,
                                 searcher._hash_includes_len,
                                 searcher._key_positions,
                                 searcher._alpha_inc,
                                 searcher._total_duplicates,
                                 searcher._alpha_size,
                                 searcher._asso_values);
          }
        else
          {
			outputter = new Output(searcher._head,
                              inputter._struct_decl,
                              inputter._struct_decl_lineno,
                              inputter._return_type,
                              inputter._struct_tag,
                              inputter._verbatim_declarations,
                              inputter._verbatim_declarations_end,
                              inputter._verbatim_declarations_lineno,
                              inputter._verbatim_code,
                              inputter._verbatim_code_end,
                              inputter._verbatim_code_lineno,
                              inputter._charset_dependent,
                              searcher._total_keys,
                              searcher._max_key_len,
                              searcher._min_key_len,
                              searcher._hash_includes_len,
                              searcher._key_positions,
                              searcher._alpha_inc,
                              searcher._total_duplicates,
                              searcher._alpha_size,
                              searcher._asso_values);
          }

      struct nbperf *_nbperf = option.nbperf();
      _nbperf->out = outputter;

      searcher.optimize ();
      outputter->update_searcher (searcher._head,
				 searcher._total_keys,
				 searcher._max_key_len,
				 searcher._min_key_len,
				 searcher._hash_includes_len,
				 searcher._key_positions,
				 searcher._alpha_inc,
				 searcher._total_duplicates,
				 searcher._alpha_size,
				 searcher._asso_values);
      list = searcher._head;

      /* Open the output file.  */
      if (option.get_output_file_name ())
        if (strcmp (option.get_output_file_name (), "-") != 0)
          if (!freopen (option.get_output_file_name (), "w", stdout))
            {
              fprintf (stderr, "Cannot open output file '%s'\n",
                       option.get_output_file_name ());
              return 1;
            }

      outputter->output ();

      /* Check for write error on stdout.  */
      exitcode = 0;
      if (fflush (stdout) || ferror (stdout))
	{
	  fprintf (stderr, "error while writing output file\n");
	  exitcode = 1;
	}

#ifdef PERF
      {
        uint64_t t1 = timer_end();
        FILE *f = fopen ("gperf.log", "a");
        fprintf(f, "%20zu %20ld\n", searcher._total_keys, (signed long)(t1 - t0));
        fclose (f);
      }
#endif

      /* Here we run the Search and Output destructors.  */

	  delete outputter;
    }

    /* Also delete the list that was allocated inside Input and reordered
       inside Search.  */
    for (KeywordExt_List *ptr = list; ptr; ptr = ptr->rest())
      {
	KeywordExt *keyword = ptr->first();
	do
	  {
	    KeywordExt *next_keyword = keyword->_duplicate_link;
	    if (!option.is_mph_algo())
	      delete[] const_cast<unsigned int *>(keyword->_selchars);
	    if (keyword->_rest != empty_string)
	      delete[] const_cast<char*>(keyword->_rest);
	    if (!(keyword->_allchars >= inputter._input
		  && keyword->_allchars < inputter._input_end))
	      delete[] const_cast<char*>(keyword->_allchars);
	    delete keyword;
	    keyword = next_keyword;
	  }
	while (keyword != NULL);
      }
    delete_list (list);

    /* Here we run the Input destructor.  */
  }

  /* Don't use exit() here, it skips the destructors.  */
  return exitcode;
}
