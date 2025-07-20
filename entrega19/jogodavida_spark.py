import sys
from pyspark import SparkContext
import time

def apply_rules(cell_state, num_neighbors):
    """Aplica as regras do Jogo da Vida a uma única célula."""
    if cell_state == 1 and (num_neighbors < 2 or num_neighbors > 3):
        return 0
    elif cell_state == 0 and num_neighbors == 3:
        return 1
    else:
        return cell_state

def get_neighbors(cell):
    """Para uma célula em (r, c), retorna uma lista de suas 8 vizinhas."""
    r, c = cell
    neighbors = []
    for i in range(-1, 2):
        for j in range(-1, 2):
            if i == 0 and j == 0:
                continue
            neighbors.append((r + i, c + j))
    return neighbors

def run_game_of_life(sc, initial_board, generations, board_size):
    """Executa a simulação do Jogo da Vida com Spark."""
    all_cells = sc.parallelize([(r, c) for r in range(board_size) for c in range(board_size)])
    live_cells_rdd = sc.parallelize(initial_board).map(lambda cell: (cell, 1))
    
    board = all_cells.map(lambda cell: (cell, 0)).leftOuterJoin(live_cells_rdd).map(
        lambda x: (x[0], x[1][1] if x[1][1] is not None else 0)
    )

    board.cache()

    print(f"--- Início da Simulação (Gerações={generations}, Tamanho={board_size}x{board_size}) ---")
    
    for gen in range(generations):
        start_time = time.time()
        
        neighbor_counts = board.filter(lambda x: x[1] == 1) \
                               .flatMap(lambda x: [(neighbor, 1) for neighbor in get_neighbors(x[0])]) \
                               .reduceByKey(lambda x, y: x + y)

        board_with_neighbors = board.leftOuterJoin(neighbor_counts).map(
            lambda x: {
                "coords": x[0],
                "state": x[1][0],
                "neighbors": x[1][1] if x[1][1] is not None else 0
            }
        )
        
        new_board = board_with_neighbors.map(lambda x: (x["coords"], apply_rules(x["state"], x["neighbors"])))
        
        new_board.cache()
        live_cell_count = new_board.filter(lambda x: x[1] == 1).count()
        board.unpersist()
        board = new_board

        end_time = time.time()
        print(f"Geração {gen + 1}: {live_cell_count} células vivas (levou {end_time - start_time:.2f}s)")

    print("--- Fim da Simulação ---")
    return board.filter(lambda x: x[1] == 1).collect()


if __name__ == "__main__":
    sc = SparkContext("local[*]", "JogoDaVidaSpark")

    board_size = 16 
    generations_to_run = 10
    
    initial_glider = [(1, 2), (2, 3), (3, 1), (3, 2), (3, 3)] 
    
    final_live_cells = run_game_of_life(sc, initial_glider, generations_to_run, board_size)

    print("\nPosição final das células vivas:")
    for cell in sorted(final_live_cells):
        print(cell)

    sc.stop()
