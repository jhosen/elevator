
/*
char * inputtoaction(char *string, char * globalbuf, int * offset, struct peer p, struct timeval * ttime){
		//strncpy( ( globalbuf+(*offset)), string, strlen(string));

		int start_i = 0;
		int end_i = 0; //cjsonendindex(globalbuf, start_i);
		char * cjsonstr = malloc(strlen(string));
		// Get all complete cjsonstrings
		//globalbuf[(*offset)+strlen(string)] = '\0';
		int i;
		for(i = 0; i <strlen(string); i++){
			if(string[i]=='<'){
				start_i = i;
			}
			if(string[i]=='>'){
				end_i = i;
				if(end_i>start_i){
					strncpy(cjsonstr, (string+start_i+1), end_i-start_i-1);
					cjsonstr[end_i-start_i-1]=  '\0';
					struct msg packet = unpack(cjsonstr);
//					if(packet.msgtype!=OPCODE_IMALIVE)
//						printf("cjsonstr: %s\n", cjsonstr);
					handle_msg(packet, ttime);
					memset(cjsonstr, 0, strlen(cjsonstr));
				}
				else{
					; // bug
				}
			}
		}
//		free(cjsonstr);
//		if(end_i < strlen(string)-1){
//			// We have fragments
//			char *fragment = malloc(strlen(string)-end_i);
//			strncpy(fragment, (string+end_i), (strlen(string)-end_i));
//
//
//			strncpy((globalbuf+(*offset)), (string+start_i), (strlen(string)-start_i) );
//						(*offset) = (*offset) +  (strlen(string)-start_i);
//						*(globalbuf+(*offset)) = '\0';
//		}
//
		return 0;

}*/
/*
char * inputtoaction(char *string, char * globalbuf, int * offset, struct peer p, struct timeval * ttime){
		strncpy( ( globalbuf+(*offset)), string, strlen(string));
		int start_i = 0;
		int end_i = cjsonendindex(globalbuf, start_i);
		char * cjsonstr = malloc(strlen(string));
		// Get all complete cjsonstrings
		while(end_i<(strlen(globalbuf)-1)){

			strncpy(cjsonstr, (globalbuf + start_i), (end_i-start_i+1));
			cjsonstr[end_i-start_i+1] = '\0';

			// We have a complete cjson string, now do the magic:
			if(end_i!=-1){
				struct msg packetin = unpack(cjsonstr);
				packetin.from = p.ip;
				handle_msg(packetin, ttime);
			}

			start_i = end_i+1;
			end_i = cjsonendindex(globalbuf, start_i);

			if(end_i == -1) // No complete cjson string left: maybe a fraction?
				break;
		}
//		if(end_i == -1){ // we have a incomplete string, and must save the fraction
//			strncpy((globalbuf+(*offset)), (string+start_i), (strlen(string)-start_i) );
//			(*offset) = (*offset) +  (strlen(string)-start_i);
//		}
//		else{
//			strncpy(cjsonstr, (globalbuf + start_i ), (end_i-start_i+1));
//			cjsonstr[end_i-start_i+1] = '\0';
//
//			// We have a complete cjson string, now do the magic:
//			struct msg packetin = unpack(cjsonstr);
//			packetin.from = p.ip;
//			handle_msg(packetin, ttime);
//			memset(cjsonstr, 0, sizeof(cjsonstr));
//
//			(*offset) = 0;
//			memset(globalbuf, 0, strlen(globalbuf));
//		}
		return globalbuf;


}*/
/*
char * inputtoaction(char *string, char * globalbuf, int * offset, struct peer p, struct timeval * ttime){
		strncpy( ( globalbuf+(*offset)), string, strlen(string));
		int start_i = 0;
		int end_i = cjsonendindex(globalbuf, start_i);
		char * cjsonstr = malloc(strlen(string));
		// Get all complete cjsonstrings
		while(end_i<(strlen(globalbuf)-1)){

			strncpy(cjsonstr, (globalbuf + start_i), (end_i-start_i+1));
			cjsonstr[end_i-start_i+1] = '\0';

			// We have a complete cjson string, now do the magic:
			if(end_i!=-1){
				struct msg packetin = unpack(cjsonstr);
				packetin.from = p.ip;
				handle_msg(packetin, ttime);
			}

			start_i = end_i+1;
			end_i = cjsonendindex(globalbuf, start_i);

			if(end_i == -1) // No complete cjson string left: maybe a fraction?
				break;
		}
		if(end_i == -1){ // we have a incomplete string, and must save the fraction
			strncpy((globalbuf+(*offset)), (string+start_i), (strlen(string)-start_i) );
			(*offset) = (*offset) +  (strlen(string)-start_i);
		}
		else{
			strncpy(cjsonstr, (globalbuf + start_i ), (end_i-start_i+1));
			cjsonstr[end_i-start_i+1] = '\0';

			// We have a complete cjson string, now do the magic:
			struct msg packetin = unpack(cjsonstr);
			packetin.from = p.ip;
			handle_msg(packetin, ttime);
			memset(cjsonstr, 0, sizeof(cjsonstr));

			(*offset) = 0;
			memset(globalbuf, 0, strlen(globalbuf));
		}
		return globalbuf;


}*/

/*
char * inputtoaction(char *string, char * globalbuf, int * offset, struct peer p, struct timeval * ttime){
		strncpy( ( globalbuf+(*offset)), string, strlen(string));
		int start_i = 0;
		int end_i = cjsonendindex(globalbuf, start_i);
		char * cjsonstr = malloc(strlen(string));
		// Get all complete cjsonstrings
		while(end_i<(strlen(globalbuf)-1)){

			strncpy(cjsonstr, (globalbuf + start_i), (end_i-start_i+1));
			cjsonstr[end_i-start_i+1] = '\0';

			// We have a complete cjson string, now do the magic:
			if(end_i!=-1){
				struct msg packetin = unpack(cjsonstr);
				packetin.from = p.ip;
				handle_msg(packetin, ttime);
			}

			start_i = end_i+1;
			end_i = cjsonendindex(globalbuf, start_i);

			if(end_i == -1) // No complete cjson string left: maybe a fraction?
				break;
		}
		if(end_i == -1){ // we have a incomplete string, and must save the fraction
			strncpy((globalbuf+(*offset)), (string+start_i), (strlen(string)-start_i) );
			(*offset) = (*offset) +  (strlen(string)-start_i);
		}
		else{
			strncpy(cjsonstr, (globalbuf + start_i ), (end_i-start_i+1));
			cjsonstr[end_i-start_i+1] = '\0';

			// We have a complete cjson string, now do the magic:
			struct msg packetin = unpack(cjsonstr);
			packetin.from = p.ip;
			handle_msg(packetin, ttime);
			memset(cjsonstr, 0, sizeof(cjsonstr));

			(*offset) = 0;
			memset(globalbuf, 0, strlen(globalbuf));
		}
		return globalbuf;


}*/

char * fun(char *string, char * globalbuf, int * offset, struct peer p, struct timeval * ttime){
		//strncpy( ( globalbuf+(*offset)), string, strlen(string));

		int start_i = 0;
		int end_i = 0; //cjsonendindex(globalbuf, start_i);
		char * cjsonstr = malloc(strlen(string));
		// Get all complete cjsonstrings
		//globalbuf[(*offset)+strlen(string)] = '\0';
		int i;
		for(i = 0; i <strlen(string); i++){
			if(string[i]=='<'){
				start_i = i;
			}
			if(string[i]=='>'){
				end_i = i;
				if(end_i>start_i){
					strncpy(cjsonstr, (string+start_i), end_i-start_i+1);
					struct msg packet = unpack(cjsonstr);
					handle_msg(packet, ttime);
				}
				else{
					; // bug
				}
			}
		}
//		free(cjsonstr);
//		if(end_i < strlen(string)-1){
//			// We have fragments
//			char *fragment = malloc(strlen(string)-end_i);
//			strncpy(fragment, (string+end_i), (strlen(string)-end_i));
//
//
//			strncpy((globalbuf+(*offset)), (string+start_i), (strlen(string)-start_i) );
//						(*offset) = (*offset) +  (strlen(string)-start_i);
//						*(globalbuf+(*offset)) = '\0';
//		}
//


}