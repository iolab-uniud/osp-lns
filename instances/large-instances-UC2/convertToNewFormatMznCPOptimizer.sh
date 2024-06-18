for file in ../*.json; do
	filename="${file%.*}"
	ECHO $filename
	../../../../../mcp.ovenschedulingalgorithm/MCP.OvenSchedulingAlgorithm/MCP.OvenSchedulingAlgorithmCLI/bin/Release/netcoreapp3.1/MCP.OvenSchedulingAlgorithmCLI.exe -i $file --cNew  --dznF $filename --specialLexW 100 >> basic_satisfiability_test.txt 
	../../../../../mcp.ovenschedulingalgorithm/MCP.OvenSchedulingAlgorithm/MCP.OvenSchedulingAlgorithmCLI/bin/Release/netcoreapp3.1/MCP.OvenSchedulingAlgorithmCLI.exe -i $file --cNew  --cCP --dznF $filename --specialLexW 100 >> basic_satisfiability_test.txt  
	sleep 1
done

$SHELL

