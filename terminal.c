#include <sys/wait.h>
#include <sys/types.h>
#include <unistd.h>
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#define QTD_TOKS_LINHA 64
#define TOK_DELIMITADOR " \t\r\n\a"
#define clear() printf("\033[H\033[J")
char *comandosString[] = {
	"irPara",
	"ajuda",
	"sair"
};
int num_comandos(){
	return sizeof(comandosString) / sizeof(char*);
}
int irPara(char **args){
	if(args[1] == NULL){
		fprintf(stderr, "Argumento esperado para \"cd\"\n");
	} else {
		if(chdir(args[1]) != 0){
			perror("Erro ao mudar de diretório");
		}
	}
	return 1;
}

int ajuda(char **args){
	int i;
	printf("Esse é o Terminal de João Pedro Lacerda, Feito através de muitas pesquisas e estudos para implementar \n");
	printf("Escreva os comandos no terminal.\nOs comandos são:\n");

	for(i = 0; i < num_comandos(); i++){
		printf(" %s\n", comandosString[i]);
	}

	return 1;
}

int sair(char **args){
	return 0;
}


int (*comandos_func[]) (char**) = {
	&irPara,
	&ajuda,
	&sair
};
char *ler_linha(void){
	char *linha = NULL;
	ssize_t tam = 0;

	getline(&linha, &tam, stdin);

	return linha;
}

char **divide_linha(char *linha){

	int toks_linha = QTD_TOKS_LINHA;
	int posicao = 0;
	char **tokens = malloc(sizeof(char*) * toks_linha);
	char *token;

	if(!tokens){
		fprintf(stderr, "erro de alocação\n");
		exit(EXIT_FAILURE);
	}

	// pega o primeiro token
	token = strtok(linha, TOK_DELIMITADOR);

	// pega os proximos tokens
	while(token!=NULL){
		tokens[posicao] = token;
		

		if(posicao >= QTD_TOKS_LINHA){
			toks_linha += QTD_TOKS_LINHA;
			tokens = realloc(tokens, toks_linha * sizeof(char*));

			if(!tokens){
				fprintf(stderr, "erro de alocação\n");
				exit(EXIT_FAILURE);
			}
		}

		token = strtok(NULL, TOK_DELIMITADOR);
		posicao++;
	}

	tokens[posicao] = NULL;
	return tokens;
}

int launch(char **args){
	pid_t paiId, wpid;
	int status;

	paiId = fork();

	if(paiId == 0){
		// Processo filho

		/* execvp recebe um nome de arquivo ou caminho de 
		um novo arquivo de programa para carregar e executar 
		como o primeiro argumento, e o segundo argumento
		recebe un array de argumentos do programa */
		if(execvp(args[0], args) == -1){          
			perror("Comando não existe. Escreva <ajuda> para conhecer os comandos.");
		}

	} else if(paiId < 0){
		// Processo deu erro
		perror("Erro ao iniciar novo processo");
	} else {
		// Processo pai

		/* O pai espera o comando do filho finalizar, ou seja,
		o waitpid() espera que o estado do processo filho mude.
		Nessa função a gente passa como parâmetros o ID do processo pai , 
		o endereço de uma variável do tipo int onde o waitpid vai colocar o status do filho 
		 e o evento que a gente quer monitorar do processo filho . */
		do{
			wpid = waitpid(paiId, &status, WUNTRACED);
		} while(!WIFEXITED(status) && !WIFSIGNALED(status));

		// WIFEXITED Verifica se o processo filho finalizou com sucesso
		// WIFSIGNALED determina se o processo filho foi encerrado porque gerou um sinal que fez com que ele fosse encerrado
	}

	return 1;
}

int executar(char **args){
	int i;

	if(args[0] == NULL){
		return 1;
	}

	for(i = 0; i < num_comandos(); i++){
		if (strcmp(args[0], "sair")==0)
		{
			return NULL;
		}else if(strcmp(args[0], comandosString[i]) == 0){
			return (*comandos_func[i])(args);
		}
	}

	return launch(args);
}



void loop(){
	char *linha;
	char **args;
	int status;
	char cwd[256];
	do{
		getcwd(cwd,sizeof(cwd));
		printf("Fale com o lacerda (%s)>",cwd);
		linha = ler_linha();
		args = divide_linha(linha);
		status = executar(args);

		free(linha);
		free(args);

	} while(status);
}

int main (int argc, char **argv){
	clear();
  printf("BEM VINDO AO TERMINAL DE LACERDINHA\n");
	loop();
	return EXIT_SUCCESS;
}
