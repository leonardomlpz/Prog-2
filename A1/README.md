## PROJETO - GERENCIADOR DE BIBLIOTECA VIRTUAL (GBV)

### 1. Autoria

* **Nome:** Leonardo Martins Lopes
* **GRR:** 20242343

---

### 2. Lista de Arquivos

* `gbv.h`: Arquivo de cabeçalho que define as estruturas de dados (`Library`, `Document`) e os protótipos das funções principais.
* `gbv.c`: Implementação das funções de manipulação da biblioteca (`gbv_create`, `gbv_open`, `gbv_add`, `gbv_remove`, `gbv_list`, `gbv_view`, `gbv_close`).
* `util.h`: Arquivo de cabeçalho para funções utilitárias.
* `util.c`: Implementação da função utilitária `format_date`.
* `main.c`: Ponto de entrada do programa. Responsável por processar os argumentos da linha de comando e chamar as funções apropriadas.
* `Makefile`: Arquivo de compilação para automatizar o processo de build do projeto.
* `README.md`: Este arquivo de documentação.

---

### 3. Dificuldades Enfrentadas e Bugs Conhecidos

#### Dificuldades Enfrentadas:

* A mudança do requisito de remoção lógica para remoção física com compactação, que aumentou significativamente a complexidade da função `gbv_remove`.
* Entender o gerenciamento do ponteiro de arquivo (`FILE*`) como um estado global.
* Corrigir o bug de "double free" causado pela chamada incorreta da função `gbv_close` dentro de `gbv_remove`.
* Compreender o funcionamento do buffer de saída (`stdout`) e a necessidade de `fflush(stdout)` para corrigir a exibição na função `gbv_view`.

#### Bugs conhecidos:

* Sem bugs conhecidos.