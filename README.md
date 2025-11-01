# Exemplo de POSIX Threads (pthreads) em C

Exemplo de uso de **POSIX Threads** no Linux, demonstrando:

- Criação de múltiplas threads trabalhadoras
- Fila de tarefas thread-safe com `mutex` e `condition variables`
- Sincronização entre produtor (main) e consumidores (threads)
- Encerramento gracioso com sinalização via `done`
- Uso de `pthread_join` para esperar finalização

---

## Compilação

```bash
gcc -o thread_pool thread_pool_example.c -lpthread
```

## Execução

```bash
./thread_pool
```

## Conceitos demonstrados

| Recurso              | Uso                                      |
|----------------------|------------------------------------------|
| `pthread_create()`   | Criação de threads                       |
| `pthread_join()`     | Espera término                           |
| `pthread_mutex_t`    | Exclusão mútua na fila                   |
| `pthread_cond_t`     | Sincronização (not_empty, not_full)      |
| Fila circular        | Estrutura de dados eficiente             |
| Encerramento seguro  | Sinalização com `done` + `broadcast`     |

---

## Requisitos

- Linux
- GCC
- Biblioteca pthread (`-lpthread`)

---

## Licença

MIT



