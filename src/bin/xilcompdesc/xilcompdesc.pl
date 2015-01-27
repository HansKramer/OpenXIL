#!/shared/xil/local/bin/perl
##
##  @(#)xilcompdesc.pl	1.1 96/12/03 
##
die "Usage:  $0 className classType files ...\n" if $#ARGV < 2;

$className = shift @ARGV;
$classType = shift @ARGV;

sub output_preamble 
{
    print "////////////////////////////////////////////////////////////////\n";
    print "//                                                            //\n";
    print "//   THIS IS AN AUTOMATICALLY GENERATED FILE FOR DESCRIBING   //\n";
    print "//     GPI IMAGE PROCESSING ROUTINES TO THE XIL LIBRARY.      //\n";
    print "//                                                            //\n";
    print "//             IT SHOULD NOT BE EDITED BY HAND.               //\n";
    print "//                                                            //\n";
    print "////////////////////////////////////////////////////////////////\n";
    print "\n";
    print "#include <xil/xilGPI.hh>\n";
    print "#include \"" . $className . ".hh\"\n";
    print "\n";
    print "XilStatus\n";
    print "$className" . "::describeMembers()\n";
    print "{\n";
    print "    XilFunctionInfo* fi;\n";
    print "\n";
}

sub output_func
{
    die "No OP specified for function $funcName\n" if $numOps == 0;

    print "    fi = XilfunctionInfo::create();\n";

    ##
    ##  Write the operations in reverse order -- bottom op first.
    ##
    for ($i = $numOps - 1; $i >= 0; $i--) {
        print "    fi->describeOp(XIL_STEP, 1, \"$ops[$i]\");\n";
    }

    ##
    ##  Set the function to call.
    ##
    print "    fi->setFunction((Xil" . $classType . "FunctionPtr)\n";
    print "                    $className" . "::" . "$funcName,\n";

    ##
    ##  Output a string that describes the function in "show_action"
    ##  notation. 
    ##
    ##  (i.e. display_SUNWcg6(copy;8()) notation)
    ##
    print "                    \"";
    for ($i = $numOps - 1; $i >= 0; $i--) {
        print "$ops[$i](";
    }
    for ($i = 0; $i < $numOps; $i++) {
        print ")";
    }
    print "\");\n";

    ##
    ##  Deal with any preprocess/postprocess routines.
    ##
    if ($preprocess) {
        print "    fi->setPreprocessFunction((Xil" . $classType . 
            "PreprocessFunctionPtr)\n";
        print "                              $className" . "::" .
            "$preprocess)\n"; 
    }

    if ($postprocess) {
        print "    fi->setPostprocessFunction((Xil" . $classType . 
            "PostprocessFunctionPtr)\n";
        print "                               $className" . "::" .
            "$postprocess)\n"; 
    }

    ##
    ##  Tell the device manager to add the specified function.
    ##
    print "    this->addFunction(fi);\n";
    print "    fi->destroy();\n";
    print "\n";
}

sub output_closure 
{
    print "    return XIL_SUCCESS;\n";
    print "}\n";
}

#######################################################################
##
##  xilcompdesc.pl implementation -- main loop
##
#######################################################################

&output_preamble;

while(<>) {
    if(/XILCONFIG/) {
        split;

        die "Invalid XILCONFIG line -- missing 'XILCONFIG:'\n"
            if $_[1] ne "XILCONFIG:";

        die "Missing '{' on XILCONFIG line\n"
            if $_[3] ne "{";

        $funcName = $_[2];

        $in_config_block = 1;
    }

    if ($in_config_block) {
        split;

        if ($_[1] eq "}") {
            &output_func();

            ##
            ##  Clear variables for next block.
            ##
            $in_config_block = 0;
            $numOps = 0;
            $preprocess = 0;
            $postprocess = 0;

            next;
        }

        ##
        ##  Split the next token on the '='
        ##
        split /=/, $_[1];

      SWITCH: {
          if ($_[0] eq "OP") {
              $ops[$numOps++] = $_[1];
              last SWITCH; 
          }

          if ($_[0] eq "PRE") {
              $preprocess = $_[1];
              last SWITCH;
          }

          if ($_[0] eq "POST") {
              $postprocess = $_[1];
              last SWITCH;
          }
      }
    }
}

&output_closure();
