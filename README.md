# assmblerSimulator
This is a school project simulating a 2 stage assembler.  
  
**input:** "assembly" files ending with .as  
**output:** enty file (.ent), external file (.ext), object file (.ob) in special base 4 "computer languge".  
  
**First pass:** the program reads a phrases text from a file, and sortes information into 3 tables - symbol, data, and code. If information can't be processed at this time it is added to a queue.  
  
**Second pass:** the program adds missing information from the queue and outputs up to 3 files: .ent for entries, .ext for externals, and .ob for complete code translation into special language.
