#!/bin/bash
# Include any argument to suppress output.  E.g.,
#    source ~jbm/jip-current/define-jip.script 1
# from startup file .bashrc

# Set the JIP_HOME environment variable to access documentation from programs.
#    e.g., xd -H
export JIP_HOME=/homes/me/jip-tools
# These aliases point to scripts that pick the correct executables.
alias jip="$JIP_HOME/bin/jip"
alias xd="$JIP_HOME/bin/display"
alias align="$JIP_HOME/bin/align"
alias glm="$JIP_HOME/bin/glm"
alias pet="$JIP_HOME/bin/pet"
alias rem="$JIP_HOME/bin/rem"

# Using any argument to this script will suppress output.
if [ $# -eq 0 ]
then
echo " "
echo "Defined JIP_HOME to be $JIP_HOME"
echo "Defined these aliases: jip, xd, align, glm, pet, rem"
echo " "
echo "Get help using:"
echo " "
echo "xd -h        or    xd -H"
echo "align -h     or    align -H"
echo "glm -h       or    glm -H"
echo "rem -h       or    rem -H"
echo "jip [return] help"
echo " "
fi
