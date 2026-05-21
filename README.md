# Documentação de Lógica — Sprint 2: Sistema Inteligente de Gerenciamento de Recarga

##  Integrantes

*   **João Marcelo de Melo e Silva** - RM: 572569
*   **Lucas Barreto Santana** - RM: 573149
*   **Pablo Renato dos Santos Sobral de Carvalho** - RM: 569894
*   **Pedro Vianna** - RM: 570747

---

### Repositório GitHub: [https://github.com/Dustyluc123/DSA-sprint-2-](https://github.com/Dustyluc123/DSA-sprint-2-)

**Curso:** Ciências da Computação  
**Instituição:** FIAP  
**Sprint:** 2

-------

Este documento apresenta a especificação técnica e arquitetural do software **EletroCharge G2**, desenvolvido em linguagem **C**. O sistema atua na camada de inteligência de borda (*Edge Computing*) para infraestruturas de recarga de veículos elétricos (EV), abordando diretamente os desafios de balanceamento de carga, faturamento flexível e telemetria industrial em conformidade com o **EV Challenge 2026 (FIAP & GoodWe)**.

1. Diretrizes da Arquitetura e Configurações Globais
O sistema opera sob restrições físicas de hardware e rede, simuladas através de constantes de pré-processamento. Estas diretrizes impedem o colapso da rede elétrica local e definem a capacidade operacional estática do eletroposto:

| Diretiva (#define) | Valor Nominal | Descrição Técnica |
| :--- | :--- | :--- |
| `MAX_VAGAS` | 3 | Limite máximo de conectores físicos operando em paralelo. |
| `POTENCIA_MAXIMA_REDE` | 15.0 kW | Capacidade térmica e elétrica do disjuntor geral do posto. |
| `POTENCIA_POR_CARREGADOR` | 7.0 kW | Demanda nominal máxima de cada carregador individual Série HCA. |

---

2. Modelagem de Dados e Estruturas
Para a persistência temporária de dados em memória volátil (RAM), o sistema utiliza um dicionário de dados modelado através de uma estrutura de dados homogênea conjugada (`struct`). Cada conector ativo gera uma instância de telemetria isolada.

```c
struct Sessao {
    int id_vaga;               // Identificador do conector fisico (1 a MAX_VAGAS)
    int ativa;                 // Flag booleano de estado (0: Livre, 1: Ocupada)
    char nome[64];             // Buffer de identificacao do motorista
    int tipo_usuario;          // Segmentacao de cliente (1: Padrao, 2: Premium)
    int hora_atual;            // Relogio operacional interno da sessao (0-23h)
    int minutos_restantes;     // Contador decrescente de tempo de carga
    double energia_consumida;  // Acumulador de energia integrada em tempo real (kWh)
    double custo_acumulado;    // Registro contabil financeiro parcial (R$)
};
```

As instâncias são alocadas estaticamente em um vetor global de estruturas denominado `vagas[MAX_VAGAS]`, emulando uma tabela de banco de dados em memória de acesso rápido.

---

3. Lógica de Negócio e Algoritmos Críticos

### 3.1. Orquestração Dinâmica de Potência (Dynamic Load Balancing)
O coração lógico do sistema reside no mecanismo de proteção contra sobrecarga. Se a demanda nominal total dos carros conectados (`carros_ativos * 7.0 kW`) superar o teto físico do local (`15.0 kW`), o software realiza uma divisão justa e matemática da potência.

> **Exemplo de cenário:** Se 3 carros conectarem simultaneamente, a demanda teórica seria de 21 kW. O sistema detecta a violação do limite e reduz dinamicamente a potência injetada por conector para **5.0 kW** (15.0 kW / 3), preservando a integridade da infraestrutura elétrica.

### 3.2. Tarifação Dinâmica de Alta Ocupação e Horários de Pico
A precificação do kWh é calculada minuto a minuto com base em uma matriz variável regulada por fatores de mercado:

*   **Ocupação Total:** Quando o posto atinge capacidade máxima (3 carros ativos), a tarifa base sofre um incremento emergencial, migrando de **R$ 0.75** para **R$ 0.95/kWh** como taxa de desincentivo a congestionamentos.
*   **Horário de Pico:** Sessões que transitam entre as **18:00h e 21:00h** recebem uma taxa adicional de **R$ 0.45 por minuto/kWh**, refletindo o custo de demanda da distribuidora local (ANEEL RE 1.000/2021).
*   **Subsídio Premium:** Clientes com classificação **Premium** desfrutam de um desconto linear de **15%** (fator de multiplicação de 0.85) sobre o valor total da tarifa apurada por minuto.

---

4. Modularização Funcional do Código
O código-fonte é estruturado em sub-rotinas especializadas com alta coesão e baixo acoplamento, facilitando futuras migrações ou integrações de APIs.

| Sub-rotina / Função | Operação Realizada | Mecanismo de Controle |
| :--- | :--- | :--- |
| `inicializar_sistema()` | Laço iterativo que limpa os blocos de memória da struct. | Define flag ativa para 0 em todos os índices. |
| `adicionar_sessao()` | Efetua o registro de novas conexões e valida inputs. | Utiliza laços 'while' para barrar estouro de limites ou dados inválidos. |
| `simular_tempo()` | Motor de passo temporal do simulador (integração de energia/custo). | Atualiza o relógio bi-dimensional e diminui contadores de carga. |
| `exibir_relatorios()` | Varre o array global traduzindo dados crus em informações legíveis. | Filtra registros ativos gerando relatórios consolidados em tela. |
| `simular_integracao_ocpp()` | Tradutor de protocolo para envio de pacotes IoT industriais. | Usa strings formatadas para gerar mock de payloads JSON legítimos. |

---

5. Comunicação e Telemetria de Borda (OCPP Mocking)
Para cumprir a exigência regulatória de interoperabilidade de rede do setor de eletromobilidade, o software simula transações nativas do protocolo **OCPP (Open Charge Point Protocol)**. A cada ciclo de sincronismo acionado, as métricas integradas de consumo em floats são serializadas em formato textual JSON padrão de mercado:

```json
[OCPP] -> {"action": "MeterValues", "connectorId": 1, "meterValue": 14.520, "user": "Lucas Barreto"}
```

Esse fluxo de saída garante que os dados brutos gerados no carregador físico possam ser consumidos por *Webhooks* de plataformas em nuvem ou disparadores automatizados (como fluxos estruturados no **n8n**) nas arquiteturas de camadas superiores.
Valor Nominal
Descrição Técnica
 
MAX_VAGAS
3
Limite máximo de conectores físicos operando em paralelo.
POTENCIA_MAXIMA_REDE
15.0 kW
Capacidade térmica e elétrica do disjuntor geral do posto.
POTENCIA_POR_CARREGADOR
7.0 kW
Demanda nominal máxima de cada carregador individual Série HCA.

2. Modelagem de Dados e Estruturas
Para a persistência temporária de dados em memória volátil (RAM), o sistema utiliza um dicionário de dados modelado através de uma estrutura de dados homogênea conjugada (struct). Cada conector ativo gera uma instância de telemetria isolada.
struct Sessao {
    int id_vaga;               // Identificador do conector fisico (1 a MAX_VAGAS)
    int ativa;                 // Flag booleano de estado (0: Livre, 1: Ocupada)
    char nome[64];             // Buffer de identificacao do motorista
    int tipo_usuario;          // Segmentacao de cliente (1: Padrao, 2: Premium)
    int hora_atual;            // Relogio operacional interno da sessao (0-23h)
    int minutos_restantes;     // Contador decrescente de tempo de carga
    double energia_consumida;  // Acumulador de energia integrada em tempo real (kWh)
    double custo_acumulado;    // Registro contabil financeiro parcial (R$)
};


As instâncias são alocadas estaticamente em um vetor global de estruturas denominado vagas[MAX_VAGAS], emulando uma tabela de banco de dados em memória de acesso rápido.
3. Lógica de Negócio e Algoritmos Críticos
3.1. Orquestração Dinâmica de Potência (Dynamic Load Balancing)
O coração lógico do sistema reside no mecanismo de proteção contra sobrecarga. Se a demanda nominal total dos carros conectados (carros_ativos * 7.0 kW) superar o teto físico do local (15.0 kW), o software realiza uma divisão justa e matemática da potência.
Exemplo de cenário: Se 3 carros conectarem simultaneamente, a demanda teórica seria de 21 kW. O sistema detecta a violação do limite e reduz dinamicamente a potência injetada por conector para 5.0 kW (15.0 kW / 3), preservando a integridade da infraestrutura elétrica.
3.2. Tarifação Dinâmica de Alta Ocupação e Horários de Pico
A precificação do kWh é calculada minuto a minuto com base em uma matriz variável regulada por fatores de mercado:
Ocupação Total: Quando o posto atinge capacidade máxima (3 carros ativos), a tarifa base sofre um incremento emergencial, migrando de R$ 0.75 para R$ 0.95/kWh como taxa de desincentivo a congestionamentos.
Horário de Pico: Sessões que transitam entre as 18:00h e 21:00h recebem uma taxa adicional de R$ 0.45 por minuto/kWh, refletindo o custo de demanda da distribuidora local (ANEEL RE 1.000/2021).
Subsídio Premium: Clientes com classificação Premium desfrutam de um desconto linear de 15% (fator de multiplicação de 0.85) sobre o valor total da tarifa apurada por minuto.
4. Modularização Funcional do Código
O código-fonte é estruturado em sub-rotinas especializadas com alta coesão e baixo acoplamento, facilitando futuras migrações ou integrações de APIs.
Sub-rotina / Função
Operação Realizada
Mecanismo de Controle
 
inicializar_sistema()
Laço iterativo que limpa os blocos de memória da struct.
Define flag ativa para 0 em todos os índices.
adicionar_sessao()
Efetua o registro de novas conexões e valida inputs.
Utiliza laços 'while' para barrar estouro de limites ou dados inválidos.
simular_tempo()
Motor de passo temporal do simulador (integração de energia/custo).
Atualiza o relógio bi-dimensional e diminui contadores de carga.
exibir_relatorios()
Varre o array global traduzindo dados crus em informações legíveis.
Filtra registros ativos gerando relatórios consolidados em tela.
simular_integracao_ocpp()
Tradutor de protocolo para envio de pacotes IoT industriais.
Usa strings formatadas para gerar mock de payloads JSON legítimos.

5. Comunicação e Telemetria de Borda (OCPP Mocking)
Para cumprir a exigência regulatória de interoperabilidade de rede do setor de eletromobilidade, o software simula transações nativas do protocolo OCPP (Open Charge Point Protocol). A cada ciclo de sincronismo acionado, as métricas integradas de consumo em floats são serializadas em formato textual JSON padrão de mercado:
[OCPP] -> {"action": "MeterValues", "connectorId": 1, "meterValue": 14.520, "user": "Lucas Barreto"}


Esse fluxo de saída garante que os dados brutos gerados no carregador físico possam ser consumidos por Webhooks de plataformas em nuvem ou disparadores automatizados (como fluxos estruturados no n8n) nas arquiteturas de camadas superiores.
