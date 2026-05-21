#include <stdio.h>

// Configuracoes do sistema
#define MAX_VAGAS 3
#define POTENCIA_MAXIMA_REDE 15.0  // kW totais que o posto aguenta
#define POTENCIA_POR_CARREGADOR 7.0 // kW maximos que um carro consegue puxar

// Estrutura do Dicionario (Struct) para armazenar os dados de cada carro
struct Sessao {
    int id_vaga;
    int ativa; // 0 para livre, 1 para ocupada
    char nome[64];
    int tipo_usuario; // 1 - Padrao, 2 - Premium
    int hora_atual;
    int minutos_restantes;
    double energia_consumida; // kWh
    double custo_acumulado;   // R$
};

// Variaveis globais do "Banco de Dados" em memoria
struct Sessao vagas[MAX_VAGAS];

// ============================================================================
// FUNCOES DE MODULARIZACAO
// ============================================================================

void inicializar_sistema() {
    for (int i = 0; i < MAX_VAGAS; i++) {
        vagas[i].id_vaga = i + 1;
        vagas[i].ativa = 0; // Todas as vagas comecam livres
    }
}

void adicionar_sessao() {
    int id = -1;
    // Procura a primeira vaga livre
    for (int i = 0; i < MAX_VAGAS; i++) {
        if (vagas[i].ativa == 0) {
            id = i;
            break;
        }
    }

    if (id == -1) {
        printf(">>> ERRO: Posto lotado! Nao ha vagas disponiveis.\n");
        return;
    }

    printf("\n--- NOVA SESSAO (VAGA %d) ---\n", vagas[id].id_vaga);
    printf("Nome do motorista: ");
    scanf(" %[^\n]", vagas[id].nome);

    printf("Tipo (1-Padrao, 2-Premium): ");
    scanf("%d", &vagas[id].tipo_usuario);
    while(vagas[id].tipo_usuario < 1 || vagas[id].tipo_usuario > 2) {
        printf("Invalido. Tipo (1-Padrao, 2-Premium): ");
        scanf("%d", &vagas[id].tipo_usuario);
    }

    printf("Hora atual (0-23): ");
    scanf("%d", &vagas[id].hora_atual);
    while(vagas[id].hora_atual < 0 || vagas[id].hora_atual > 23) {
        printf("Invalido. Hora (0-23): ");
        scanf("%d", &vagas[id].hora_atual);
    }

    printf("Duracao em minutos: ");
    scanf("%d", &vagas[id].minutos_restantes);

    vagas[id].energia_consumida = 0.0;
    vagas[id].custo_acumulado = 0.0;
    vagas[id].ativa = 1; // Ocupa a vaga

    printf(">>> Sessao iniciada na vaga %d com sucesso!\n", vagas[id].id_vaga);
}

void simular_tempo() {
    int minutos_avanco;
    printf("\nQuantos minutos deseja avancar no tempo? ");
    scanf("%d", &minutos_avanco);

    // Conta quantos carros estao carregando simultaneamente
    int carros_ativos = 0;
    for (int i = 0; i < MAX_VAGAS; i++) {
        if (vagas[i].ativa == 1) carros_ativos++;
    }

    if (carros_ativos == 0) {
        printf("Nenhum carro conectado no momento.\n");
        return;
    }

    // CONTROLE DE DEMANDA (A Inteligencia do Sistema)
    double potencia_disponivel_por_carro = POTENCIA_POR_CARREGADOR;
    double demanda_total = carros_ativos * POTENCIA_POR_CARREGADOR;
    
    if (demanda_total > POTENCIA_MAXIMA_REDE) {
        // Divide a energia da rede igualmente para nao desarmar o disjuntor
        potencia_disponivel_por_carro = POTENCIA_MAXIMA_REDE / carros_ativos;
        printf(">>> ALERTA: Alta demanda! Potencia reduzida para %.2f kW por carro.\n", potencia_disponivel_por_carro);
    } else {
        printf(">>> STATUS: Potencia maxima (%.2f kW) entregue aos veiculos.\n", potencia_disponivel_por_carro);
    }

    // TARIFAÇÃO DINAMICA
    double tarifa_base = 0.75;
    if (carros_ativos == MAX_VAGAS) {
        tarifa_base = 0.95; // Taxa de alta demanda (Posto lotado)
    }

    // Processa os minutos
    for (int m = 0; m < minutos_avanco; m++) {
        for (int i = 0; i < MAX_VAGAS; i++) {
            if (vagas[i].ativa == 1) {
                // Atualiza relogio
                if (m > 0 && m % 60 == 0) {
                    vagas[i].hora_atual = (vagas[i].hora_atual + 1) % 24;
                }

                // Injeta energia (kwh = potencia em kW * tempo em horas)
                double energia_minuto = potencia_disponivel_por_carro * (1.0 / 60.0);
                vagas[i].energia_consumida += energia_minuto;

                // Calcula custo com base no horario (Pico vs Normal)
                double tarifa_minuto = tarifa_base;
                if (vagas[i].hora_atual >= 18 && vagas[i].hora_atual <= 21) {
                    tarifa_minuto += 0.45; // Adicional de horario de pico
                }

                // Aplica desconto para premium
                if (vagas[i].tipo_usuario == 2) {
                    tarifa_minuto = tarifa_minuto * 0.85; // 15% off
                }

                vagas[i].custo_acumulado += energia_minuto * tarifa_minuto;
                vagas[i].minutos_restantes--;

                // Finaliza a sessao se o tempo acabar
                if (vagas[i].minutos_restantes <= 0) {
                    vagas[i].ativa = 0;
                    printf(">>> AVISO: Recarga do(a) %s (Vaga %d) foi concluida!\n", vagas[i].nome, vagas[i].id_vaga);
                }
            }
        }
    }
    printf("Simulacao avancada em %d minutos.\n", minutos_avanco);
}

void exibir_relatorios() {
    printf("\n========================================\n");
    printf("        RELATORIO DE VAGAS ATIVAS       \n");
    printf("========================================\n");
    int encontrou = 0;
    for (int i = 0; i < MAX_VAGAS; i++) {
        if (vagas[i].ativa == 1) {
            encontrou = 1;
            printf("Vaga %d | %s | %s\n", vagas[i].id_vaga, vagas[i].nome, (vagas[i].tipo_usuario == 1) ? "Padrao" : "Premium");
            printf(" -> Restam: %d min | Energia: %.2f kWh | Custo atual: R$ %.2f\n", 
                   vagas[i].minutos_restantes, vagas[i].energia_consumida, vagas[i].custo_acumulado);
            printf("----------------------------------------\n");
        }
    }
    if (encontrou == 0) printf("Nenhum veiculo carregando agora.\n");
}

void simular_integracao_ocpp() {
    printf("\n[SISTEMA] Disparando Webhook de telemetria...\n");
    for (int i = 0; i < MAX_VAGAS; i++) {
        if (vagas[i].ativa == 1) {
            // Simulacao de um payload JSON sendo enviado para a nuvem
            printf("[OCPP] -> {\"action\": \"MeterValues\", \"connectorId\": %d, \"meterValue\": %.3f, \"user\": \"%s\"}\n", 
                   vagas[i].id_vaga, vagas[i].energia_consumida, vagas[i].nome);
        }
    }
    printf("[SISTEMA] Dados enviados para a plataforma central com sucesso (HTTP 200 OK).\n");
}

// ============================================================================
// MAIN - LOOP DE INTERATIVIDADE
// ============================================================================
int main() {
    int opcao;
    inicializar_sistema();

    do {
        printf("\n========================================\n");
        printf("  SISTEMA INTELIGENTE ELETROCHARGE G2   \n");
        printf("========================================\n");
        printf("1. Conectar novo veiculo\n");
        printf("2. Simular avanco do tempo\n");
        printf("3. Exibir relatorios em tempo real\n");
        printf("4. Sincronizar dados (Simular OCPP/API)\n");
        printf("5. Encerrar sistema\n");
        printf("========================================\n");
        printf("Escolha uma opcao: ");
        scanf("%d", &opcao);

        switch (opcao) {
            case 1: adicionar_sessao(); break;
            case 2: simular_tempo(); break;
            case 3: exibir_relatorios(); break;
            case 4: simular_integracao_ocpp(); break;
            case 5: printf("Desligando sistema...\n"); break;
            default: printf("Opcao invalida. Tente novamente.\n");
        }
    } while (opcao != 5);

    return 0;
}