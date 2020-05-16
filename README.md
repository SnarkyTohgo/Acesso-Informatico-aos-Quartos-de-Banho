#Acesso informático aos Quartos de Banho


## Objetivos

    Criar programas multithread;
    Promover a intercomunicação entre processos através de canais com nome (named pipes ou FIFOs);
    Evitar conflitos entre entidades concorrentes, por via de mecanismos de sincronização;

## Utilização

    ./Q2 <-t secs> [-l nstalls] [-n nthreads] fifoname
    ./U2 <-t secs> fifoname

## Programa do cliente (Un)

    Por forma a promover race conditions o programa Un lança threads continuamente em cada 5ms, 
    e corre até terminar o tempo de execução dado como input pelo utilizador, ou até o quarto de 
    banho fechar, por isso é importante iniciar o programa do servidor e só depois iniciar o do
    cliente.

    Em cada nova thread dá-se um novo pedido:

        1. Inicialmente a thread irá compôr uma mensagem com as variáveis do inunciado 
        no formato "[n, pid, tid, duration, pl]";

        2. O FIFO público dado pelo utilizador irá ser, seguidamente, aberto se tal for possível,
        caso contrário o cliente será notificado com a operação CLSD e termina-se o ciclo 
        que gera novas threads;

        3. Quando ocorrer uma ligação com o FIFO público será escrita uma nova mensagem e é fechada
        a ligação;

        4. Depois obtém-se uma resposta do servidor ao criar um FIFO privado para comunicação com o mesmo;

        5. Se não for possível obter uma resposta o cliente será notificado com a operação FAILD, caso 
        contrário a thread irá verificar a posição e a duração que vieram do servidor. Se ambas forem -1
        quer dizer que o tempo de utilização do cliente vai ultrapassar o tempo de utilização do quarto 
        de banho e é transmitida uma notificação CLOSD ao utilizador, se não, o cliente recebe a notificação
        IAMIN;

        6. Concluindo, a thread fechará o FIFO privado e este é destruído.


## Programa do servidor (Un)

    O programa Qn lança threads por cada pedido dos clientes. Cria-se um FIFO público cujo nome é dado como
    input do utilizador, o programa abre-o e lê-o continuamente e quando receber um pedido cria uma nova thread.
    
    Em cada thread ocorre a seguinte sequência de passos:

        1. Extração da mensagem enviada pelo cliente no formato [n, pid, tid, duration, pl] e notificação RCVD. Posterior leitura da mensagem, a variável responsável pelo controlo dos lugares é alterada. Esta está protegida por processos de sincronização para que o incremento seja sequencial. Se os lugares forem limitados o cliente vai ocupar um lugar numa fila (tal como aconteceria num quarto de banho real).

        2. Abertura do FIFO privado fornecido pelo cliente. Se tal não for possível o servidor recebe uma
        notificação GAVUP. O número máximo de tentativas para abertura é 5.

        3. A thread verifica se a duração que o cliente quer não ultrapassa o tempo de execução do Quarto de Banho.
        Se não ultrapassar, compõe uma mensagem com o devido lugar na fila e duração de utilização e dá-se uma 
        notificação ENTER. Caso contrário, a mensagem irá retornar a duração e o lugar do cliente como -1 e 
        será enviada uma notificação 2LATE;

        4. Escrita da mensagem composta no FIFO privado e encerramento do mesmo;

        5. A thread espera o tempo da duração do cliente e dá-se uma notificação TIMUP. Se houver um número máximo de lugares o servidor desocupa o lugar.


Deste modo, independentemente do seu tempo de duração, o pedido de um cliente é atendido, mesmo que este tenha 
sido feito anteriormente ao fecho da casa de banho. O FIFO só é terminado depois de responder a todos os pedidos 
feitos até ao seu fecho.
