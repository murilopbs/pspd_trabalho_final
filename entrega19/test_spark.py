from pyspark import SparkContext
import time

print("--- [PASSO 1] Script de teste iniciado ---")

try:
    # Inicializa o Spark. Se travar aqui, o problema é com Java ou a configuração do Spark.
    print("--- [PASSO 2] Criando o SparkContext... ---")
    sc = SparkContext("local[*]", "SparkTest")
    print("--- [PASSO 3] SparkContext criado com sucesso! ---")

    # Cria um RDD simples. Esta é uma transformação "preguiçosa", deve ser instantânea.
    data = [1, 2, 3, 4, 5, 6, 7, 8]
    rdd = sc.parallelize(data)
    print("--- [PASSO 4] RDD criado na memória. ---")

    # Primeira AÇÃO. Isso força o Spark a realmente fazer um trabalho.
    # Se travar aqui, o problema está na execução das tarefas.
    element_count = rdd.count()
    print(f"--- [PASSO 5] Ação .count() executada. O RDD tem {element_count} elementos. ---")

    # Uma transformação simples.
    mapped_rdd = rdd.map(lambda x: x * 2)
    print("--- [PASSO 6] Transformação .map() definida. ---")

    # AÇÃO final. Coleta os resultados para o driver.
    results = mapped_rdd.collect()
    print("--- [PASSO 7] Ação .collect() executada. ---")
    print("\n\n*** TESTE CONCLUÍDO COM SUCESSO! ***")
    print(f"Resultado: {results}")


except Exception as e:
    print(f"\n\n*** OCORREU UM ERRO DURANTE O TESTE! ***")
    print(f"Erro: {e}")

finally:
    # Para o Spark. Se você vir esta mensagem, o programa terminou.
    sc.stop()
    print("--- [PASSO 8] SparkContext finalizado. ---")
