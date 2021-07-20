#include <fcntl.h>
#include <stdio.h>
#include <unistd.h>
#include <stdlib.h>
#include <sys/wait.h>
#include <sys/types.h>
#include <string.h>


int main()
{

  char* token;
  char comando[500];
  char **matriz_cmd;
  matriz_cmd= (char**)calloc(20*sizeof(char**),1);
  int n,cont=0,verificaPipe=0,cont_cmd=0,indices_cmd[20],arqIn,arqOut,fd[100][2],aux;
  pid_t PID;

  printf("Shell> ");

  scanf("%[^\n]s",comando); // lê os comandos
  token = strtok (comando, " ");// pega os tokens separados por espaço

  for(int i=0; i<100;i++)
    pipe(fd[i]); //cria pipes

  if (token){//existe token?
    while (token != NULL){//Enquanto há token pra ler
      matriz_cmd[cont]= (char*)calloc(sizeof(token)* sizeof(char*),1 );//linha da matriz no tamanho de caracteres do token
      if(verificaPipe==0){ //token é uma aplicação
        strcpy(matriz_cmd[cont],token);//copia token na matriz
        indices_cmd[cont_cmd]=cont;//copia indice da matriz que contem aplicação
        cont_cmd++;//aumento do num de aplicações
        verificaPipe=1;//prox token n é aplicação
      }else{//token é parametro ou operador
        if (strcmp(token, "|")==0){//se for operador pipe
          matriz_cmd[cont]=NULL;
          verificaPipe=0;//prox token é uma aplicação
        }else{
          strcpy(matriz_cmd[cont],token);//é um parametro
        }
      }
      token= strtok(NULL," ");//pega proximo token
      cont++;//prox posição da matriz
    }
  }else{
    printf("Nao tem comando");//n tem token
  }

  for (int i=0; i<cont_cmd; i++)
  {
    printf("Executando comando %s\n", matriz_cmd[indices_cmd[i]]);
    PID= fork();

    if (PID==0)//processo filho
    {
      n=indices_cmd[i];//n recebe posição da matriz q ta c/aplicação
      while(matriz_cmd[n] != NULL )//laço que funciona enquanto n for o fim da matriz ou pipe
      {
        if (strcmp(matriz_cmd[n],"<")==0)//Arq entrada
        {
          arqIn= open(matriz_cmd[n+1], O_RDONLY, 0644 );//644 dono pode le escrever,demais ler
          close(STDIN_FILENO);
          dup2(arqIn, STDIN_FILENO);
          matriz_cmd[n]=NULL;
          printf("Abrindo arquivo %s\n",matriz_cmd[n+1]);
        }
        else if(strcmp(matriz_cmd[n],">")==0)//arq saida
        {
          printf("Sobrescrevendo/Criando arquivo %s\n",matriz_cmd[n+1]);
          arqOut= open(matriz_cmd[n+1], O_CREAT | O_RDWR | O_TRUNC, 0644);
          close(STDOUT_FILENO);
          dup2(arqOut,STDOUT_FILENO);
          matriz_cmd[n]=NULL;
        }
        else if(strcmp(matriz_cmd[n], ">>")==0)
        {
          printf("Abrindo arquivo %s\n",matriz_cmd[n+1]);
          arqOut= open(matriz_cmd[n+1], O_CREAT | O_RDWR | O_APPEND, 0644 );
          close(STDOUT_FILENO);
          dup2(arqOut,STDOUT_FILENO);
          matriz_cmd[n]=NULL;
          printf("Abrindo arquivo %s\n",matriz_cmd[n+1]);

        }
        n++;
        
      }
      if(i != 0)//aplicação seguida do pipe
      {
        close(fd[i-1][1]);
        dup2(fd[i-1][0],STDOUT_FILENO);
        close(fd[i-1][0]);
      }
      if (i!=cont_cmd-1)//n é ultimo comando
      {
        close(fd[i][0]);
        dup2(fd[i][1],STDOUT_FILENO);
        close(fd[i][1]);
      }
      execvp(matriz_cmd[indices_cmd[i]],&matriz_cmd[indices_cmd[i]]);
      close(arqOut);
      close(arqIn);

    }else //processo Pai
    {
      if (i>0)
      {
        close(fd[i-1][0]);
        close(fd[i-1][1]);
      }
      waitpid(-1,NULL,0);

    }


  }
  printf("\n__Matriz de comandos__\n");
  for (int i=0; i<=cont;i++){
    aux=0;
    printf("linha %d:\t%s",i,matriz_cmd[i]);
    for(int j=0;j<=cont_cmd;j++){
      if( (i==indices_cmd[j]) && (aux==0) ){
        printf("\t\t\t-Eh uma nova aplicação.\n");
        aux=1;
      }else if( (matriz_cmd[i]==NULL) && (aux==0)){
        printf("\t\t\t-Fim desta aplicacao.\n");
        aux=1;
      }else if (aux==0)
      {
        printf("\t\t\t-Eh um parametro de aplicacao.\n");
        aux=1;
      }
      
    }
  }
}


