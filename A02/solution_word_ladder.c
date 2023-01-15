//
// AED, November 2022 (Tomás Oliveira e Silva)
//
// Second practical assignement (speed run)
//
// Place your student numbers and names here
//    N. Mec. 108579 Name : Anderson Lourenco --> PC2
//    N. Mec. 108796 Name : Sara Almeida --> PC3
//    N. Mec. 108215 Name : Hugo Correia --> PC1
//
// Do as much as you can
//   1) MANDATORY: complete the hash table code
//      *) hash_table_create (done)
//      *) hash_table_grow 
//      *) hash_table_free (done)
//      *) find_word
//      +) add code to get some statistical data about the hash table
//   2) HIGHLY RECOMMENDED: build the - (including union-find data) -- use the similar_words function...
//      *) find_representative
//      *) add_edge
//   3) RECOMMENDED: implement breadth-first search in the graph
//      *) breadh_first_search
//   4) RECOMMENDED: list all words belonginh to a connected component
//      *) breadh_first_search
//      *) list_connected_component
//   5) RECOMMENDED: find the shortest path between to words
//      *) breadh_first_search
//      *) path_finder
//      *) test the smallest path from bem to mal
//         [ 0] bem
//         [ 1] tem
//         [ 2] teu
//         [ 3] meu
//         [ 4] mau
//         [ 5] mal
//      *) find other interesting word ladders
//   6) OPTIONAL: compute the diameter of a connected component and list the longest word chain
//      *) breadh_first_search
//      *) connected_component_diameter
//   7) OPTIONAL: print some statistics about the graph
//      *) graph_info
//   8) OPTIONAL: test for memory leaks
//

#include <stdio.h>




#include <stdlib.h>
#include <string.h>


//
// static configuration
//

#define _max_word_size_  32


//
// data structures (SUGGESTION --- you may do it in a different way)
//

typedef struct adjacency_node_s  adjacency_node_t;
typedef struct hash_table_node_s hash_table_node_t;
typedef struct hash_table_s      hash_table_t;

struct adjacency_node_s
{
  adjacency_node_t *next;            // link to th enext adjacency list node
  hash_table_node_t *vertex;         // the other vertex
};

struct hash_table_node_s
{
  // the hash table data
  char word[_max_word_size_];        // the word
  hash_table_node_t *next;           // next hash table linked list node
  // the vertex data
  adjacency_node_t *head;            // head of the linked list of adjancency edges
  int visited;                       // visited status (while not in use, keep it at 0)
  hash_table_node_t *previous;       // breadth-first search parent
  // the union find data
  hash_table_node_t *representative; // the representative of the connected component this vertex belongs to
  int number_of_vertices;            // number of vertices of the conected component (only correct for the representative of each connected component)
  int number_of_edges;               // number of edges of the conected component (only correct for the representative of each connected component)
};

struct hash_table_s
{
  unsigned int hash_table_size;      // the size of the hash table array
  unsigned int number_of_entries;    // the number of entries in the hash table
  unsigned int number_of_edges;      // number of edges (for information purposes only)
  hash_table_node_t **heads;         // the heads of the linked lists
};



//
// allocation and deallocation of linked list nodes (done)
//

static adjacency_node_t *allocate_adjacency_node(void)
{
  adjacency_node_t *node;

  node = (adjacency_node_t *)malloc(sizeof(adjacency_node_t));
  if(node == NULL)
  {
    fprintf(stderr,"allocate_adjacency_node: out of memory\n");
    exit(1);
  }
  return node;
}

static void free_adjacency_node(adjacency_node_t *node)
{
  free(node);
}

static hash_table_node_t *allocate_hash_table_node(void)
{
  hash_table_node_t *node;

  node = (hash_table_node_t *)malloc(sizeof(hash_table_node_t));
  if(node == NULL)
  {
    fprintf(stderr,"allocate_hash_table_node: out of memory\n");
    exit(1);
  }
  return node;
}

static void free_hash_table_node(hash_table_node_t *node)
{
  free(node);
}


//
// hash table stuff (mostly to be done)
//

unsigned int crc32(const char *str)
{
  static unsigned int table[256];
  unsigned int crc;

  if(table[1] == 0u) // do we need to initialize the table[] array?
  {
    unsigned int i,j;

    for(i = 0u;i < 256u;i++)
      for(table[i] = i,j = 0u;j < 8u;j++)
        if(table[i] & 1u)
          table[i] = (table[i] >> 1) ^ 0xAED00022u; // "magic" constant
        else
          table[i] >>= 1;
  }
  crc = 0xAED02022u; // initial value (chosen arbitrarily)
  while(*str != '\0')
    crc = (crc >> 8) ^ table[crc & 0xFFu] ^ ((unsigned int)*str++ << 24);
  return crc;
}

//function that prints the hash table



static hash_table_t *hash_table_create(void)
{
  hash_table_t *hash_table;
  unsigned int i;

  hash_table = (hash_table_t *)malloc(sizeof(hash_table_t));
  if(hash_table == NULL)
  {
    fprintf(stderr,"create_hash_table: out of memory\n");
    exit(1);
  }
  //
  // complete this
  hash_table->hash_table_size = 3000507;
  hash_table->number_of_entries = 0;
  hash_table->number_of_edges = 0;
  hash_table->heads = (hash_table_node_t **)malloc(hash_table->hash_table_size * sizeof(hash_table_node_t *));
  if (hash_table->heads == NULL) {
    fprintf(stderr,"create_hash_table: out of memory\n");
    exit(1);
  }
  for (i = 0; i < hash_table->hash_table_size; i++) {
    hash_table->heads[i] = NULL;
  }
  return hash_table;
}


static void hash_table_grow(hash_table_t *hash_table)
{
  // Create a new, larger hash table
  unsigned int new_size = hash_table->hash_table_size * 2;
  hash_table_node_t **new_heads = (hash_table_node_t **)calloc(new_size, sizeof(hash_table_node_t *));
  if (new_heads == NULL)
  {
    // Handle allocation failure
    fprintf(stderr, "hash_table_grow: out of memory\n");
    exit(1);
  }

  // Re-hash all of the entries in the old table into the new one
  for (unsigned int i = 0; i < hash_table->hash_table_size; i++)
  {
    hash_table_node_t *node = hash_table->heads[i];
    while (node != NULL)
    {
      // Calculate the new hash value for the word
      unsigned int hash = crc32(node->word) % new_size;

      // Add the node to the new hash table
      node->next = new_heads[hash];
      new_heads[hash] = node;

      // Move to the next node in the linked list
      node = node->next;
    }
  }

  // Free the old array of hash table heads
  free(hash_table->heads);

  // Update the hash table with the new size and heads array
  hash_table->hash_table_size = new_size;
  hash_table->heads = new_heads;
}


static void hash_table_free(hash_table_t *hash_table)
{
  // Free all nodes in the hash table
  for (int i = 0; i < hash_table->hash_table_size; i++)
  {
    hash_table_node_t *node = hash_table->heads[i];
    while (node != NULL)
    {
      hash_table_node_t *next_node = node->next;

      // Free all adjacency list nodes for this hash table node
      adjacency_node_t *adj_node = node->head;
      while (adj_node != NULL)
      {
        adjacency_node_t *next_adj_node = adj_node->next;
        free_adjacency_node(adj_node);
        adj_node = next_adj_node;
      }

      free_hash_table_node(node);
      node = next_node;
    }
  }

  // Free the array of hash table heads
  free(hash_table->heads);

  // Free the hash table itself
  free(hash_table);
}

// funtion that prints the hash table
static void hash_table_print(hash_table_t *hash_table)
{
  int i, count;

  for (int i = 0; i < hash_table->hash_table_size; i++)
  {
    hash_table_node_t *node = hash_table->heads[i];
    adjacency_node_t *adj_node = node->head;
    while (node != NULL)
    {
      printf("%s\n", node->word);

      count=0;
      while(adj_node != NULL)
      {
        count = count+1;
        adj_node = adj_node->next;
      }
      
      fprintf("%d",count);
      printf("\n");
      node = node->next;
    }
  }
}

static hash_table_node_t *find_word(hash_table_t *hash_table, const char *word, int insert_if_not_found)
{
  unsigned int i = crc32(word) % hash_table->hash_table_size;
  hash_table_node_t *node = hash_table->heads[i];
  while (node != NULL)
  {
    if (strcmp(node->word, word) == 0)
    {
      // Word found, return the node
      return node;
    }
    node = node->next;
  }
  if (insert_if_not_found && strlen(word) < _max_word_size_)
  {
    // Word not found, insert it
    node = allocate_hash_table_node();
    if (node == NULL)
    {
      // Memory allocation failed
      return NULL;
    }
    strcpy(node->word, word);
    node->next = hash_table->heads[i];
    node->representative = node;
    node->previous = NULL;
    node->number_of_vertices = 1;
    node->number_of_edges = 0;
    node-> visited = 0;
    node->head = NULL;
    hash_table->heads[i] = node;
    hash_table->number_of_entries++;

    if (hash_table->number_of_entries > hash_table->hash_table_size*0.75)
    {
      // Hash table is getting full, resize it
      hash_table_grow(hash_table);
    }
  }
  else
  {
    // Word not found and insert_if_not_found flag is not set
    return NULL;
  }
  return node;
}




//
// add edges to the word ladder graph (mostly do be done)
//

static hash_table_node_t *find_representative(hash_table_node_t *node)
{
  hash_table_node_t *representative, *next_node;

  representative = node;
  while (representative != representative->representative)
  {
    representative = representative->representative;
  }

  // Path compression optimization
  next_node = node;
 for (next_node = node; next_node != representative; next_node = next_node->representative)
  {
    hash_table_node_t *temp = next_node->representative;
    next_node->representative = representative;
    next_node = temp;
  }

  return representative;
}


static void add_edge(hash_table_t *hash_table, hash_table_node_t *from, const char *word)
{
    hash_table_node_t *to, *from_representative, *to_representative;
    adjacency_node_t *link_from, *link_to;

    from_representative = find_representative(from);
    to = find_word(hash_table, word, 0);

    if (to == NULL || to == from)
        return;

    to_representative = find_representative(to);
    if (from_representative == to_representative)
    {
        from_representative->number_of_vertices++;
    }
    else if (from_representative->number_of_vertices < to_representative->number_of_vertices)
    {
        from_representative->representative = to_representative;
        to_representative->number_of_vertices += from_representative->number_of_vertices;
        to_representative->number_of_edges += from_representative->number_of_edges;
    }
    else
    {
        to_representative->representative = from_representative;
        from_representative->number_of_vertices += to_representative->number_of_vertices;
        from_representative->number_of_edges += to_representative->number_of_edges;
    }

    link_from = allocate_adjacency_node();
    link_to = allocate_adjacency_node();

    if (link_from == NULL || link_to == NULL)
    {
        fprintf(stderr, "add_edge: out of memory\n");
        exit(1);
    }

    link_from->vertex = to;
    link_from->next = from->head;
    from->head = link_from;

    link_to->vertex = from;
    link_to->next = to->head;
    to->head = link_to;

    from_representative->number_of_edges++;
    to_representative->number_of_edges++;
    hash_table->number_of_edges++;
    return;
}



//
// generates a list of similar words and calls the function add_edge for each one (done)
//
// man utf8 for details on the uft8 encoding
//

static void break_utf8_string(const char *word,int *individual_characters)
{
  int byte0,byte1;

  while(*word != '\0')
  {
    byte0 = (int)(*(word++)) & 0xFF;
    if(byte0 < 0x80)
      *(individual_characters++) = byte0; // plain ASCII character
    else
    {
      byte1 = (int)(*(word++)) & 0xFF;
      if((byte0 & 0b11100000) != 0b11000000 || (byte1 & 0b11000000) != 0b10000000)
      {
        fprintf(stderr,"break_utf8_string: unexpected UFT-8 character\n");
        exit(1);
      }
      *(individual_characters++) = ((byte0 & 0b00011111) << 6) | (byte1 & 0b00111111); // utf8 -> unicode
    }
  }
  *individual_characters = 0; // mark the end!
}

static void make_utf8_string(const int *individual_characters,char word[_max_word_size_])
{
  int code;

  while(*individual_characters != 0)
  {
    code = *(individual_characters++);
    if(code < 0x80)
      *(word++) = (char)code;
    else if(code < (1 << 11))
    { // unicode -> utf8
      *(word++) = 0b11000000 | (code >> 6);
      *(word++) = 0b10000000 | (code & 0b00111111);
    }
    else
    {
      fprintf(stderr,"make_utf8_string: unexpected UFT-8 character\n");
      exit(1);
    }
  }
  *word = '\0';  // mark the end
}

static void similar_words(hash_table_t *hash_table,hash_table_node_t *from)
{
  static const int valid_characters[] =
  { // unicode!
    0x2D,                                                                       // -
    0x41,0x42,0x43,0x44,0x45,0x46,0x47,0x48,0x49,0x4A,0x4B,0x4C,0x4D,           // A B C D E F G H I J K L M
    0x4E,0x4F,0x50,0x51,0x52,0x53,0x54,0x55,0x56,0x57,0x58,0x59,0x5A,           // N O P Q R S T U V W X Y Z
    0x61,0x62,0x63,0x64,0x65,0x66,0x67,0x68,0x69,0x6A,0x6B,0x6C,0x6D,           // a b c d e f g h i j k l m
    0x6E,0x6F,0x70,0x71,0x72,0x73,0x74,0x75,0x76,0x77,0x78,0x79,0x7A,           // n o p q r s t u v w x y z
    0xC1,0xC2,0xC9,0xCD,0xD3,0xDA,                                              // Á Â É Í Ó Ú
    0xE0,0xE1,0xE2,0xE3,0xE7,0xE8,0xE9,0xEA,0xED,0xEE,0xF3,0xF4,0xF5,0xFA,0xFC, // à á â ã ç è é ê í î ó ô õ ú ü
    0
  };
  int i,j,k,individual_characters[_max_word_size_];
  char new_word[2 * _max_word_size_];

  break_utf8_string(from->word,individual_characters);
  for(i = 0;individual_characters[i] != 0;i++)
  {
    k = individual_characters[i];
    for(j = 0;valid_characters[j] != 0;j++)
    {
      individual_characters[i] = valid_characters[j];
      make_utf8_string(individual_characters,new_word);
      // avoid duplicate cases
      if(strcmp(new_word,from->word) > 0)
        add_edge(hash_table,from,new_word);
    }
    individual_characters[i] = k;
  }
}

static int breadth_first_search(int maximum_number_of_vertices, hash_table_node_t **list_of_vertices, hash_table_node_t *origin, hash_table_node_t *goal) {
  int r = 0, w = 1;
  list_of_vertices[0] = origin;
  origin->previous = NULL;
  origin->visited = 1;
  int found = 0;
  while (r != w) {
    adjacency_node_t *node = list_of_vertices[r++]->head;
    if (found) {
      break;
    }
    while (node != NULL) {
      if (node->vertex->visited == 0) {
        node->vertex->visited = 1;
        node->vertex->previous = list_of_vertices[r - 1];
        list_of_vertices[w++] = node->vertex;
        if (node->vertex == goal) {
          found = 1;
          break;
        }
      }
      node = node->next;
    }
  }
  for (int i = 0; i < w; i++) {
    list_of_vertices[i]->visited = 0;
  }
  return w;
}



//
// list all vertices belonging to a connected component (complete this)
//

static void list_connected_component(hash_table_t *hash_table, const char *word) {
  // Find the node for the given word
  hash_table_node_t *node = find_word(hash_table, word, 0);
  int v = 0;
  if (node == NULL) {
    // The given word does not exist in the hash table
    printf("The word '%s' does not exist in the hash table.\n", word);
    return;
  }

  // Find the representative node for the connected component containing the given node
  hash_table_node_t *representative = find_representative(node);
  hash_table_node_t **list_of_vertices = malloc(representative->number_of_vertices * sizeof(hash_table_node_t *));
  printf("The connected component containing '%s' has %d vertices.\n", word, representative->number_of_vertices);

  int number_of_vertices = breadth_first_search(hash_table->number_of_entries, list_of_vertices, node, NULL);
  // Print all of the words belonging to the connected component
  for (unsigned int i = 0; i < number_of_vertices; i++) {
    printf("%s\n", list_of_vertices[i]->word);
    v++;
  }
  printf("\nNumber of vertices: %d\n", v);
  printf("Number of edges: %d\n", representative->number_of_edges);
  free(list_of_vertices);
}



//
// compute the diameter of a connected component (optional)
//

//static int largest_diameter;
//static hash_table_node_t **largest_diameter_example;

// static int connected_component_diameter(hash_table_node_t *node)
// {
//   int diameter;

//   int max_distance = 0;
//   queue_t *queue = queue_create();
//   hash_table_node_t *curr;
//   hash_table_node_t *neighbor;

//   queue_enqueue(queue, node);
//   while (!queue_isempty(queue)) {
//     curr = queue_dequeue(queue);

//     for (int i = 0; i < curr->numNeighbors; i++) {
//       neighbor = curr->neighbors[i];

//       if (neighbor->visited == false) {
//         neighbor->visited = true;

//         int distance = curr->distance + 1;

//         if (distance > max_distance) {
//           max_distance = distance;
//         }

//         neighbor->distance = distance;

//         queue_enqueue(queue, neighbor);
//       }
//     } 
//   } 
// diameter = max_distance;

//   return diameter;
// }
static int connected_component_diameter(hash_table_node_t *node)
{
    int diameter = 0;
    int maximum_number_of_vertices = find_representative(node)->number_of_vertices;
    hash_table_node_t **list_of_vertices = (hash_table_node_t **)malloc(maximum_number_of_vertices * sizeof(hash_table_node_t *));

    if (list_of_vertices == NULL) {
        fprintf(stderr, "connected_component_diameter: out of memory\n");
        exit(1);
    }

    int n = breadth_first_search(maximum_number_of_vertices, list_of_vertices, node, NULL);
    hash_table_node_t *farthest = list_of_vertices[n - 1];

    int m = breadth_first_search(maximum_number_of_vertices, list_of_vertices, farthest, NULL);

    for (int i = 0; i < m; i++) {
        int temp_diameter = 0;
        hash_table_node_t *p = list_of_vertices[i];
        while (p != NULL) {
            temp_diameter++;
            p = p->previous;
        }
        if (temp_diameter > diameter) {
          diameter = temp_diameter;  
        }

    }
    free(list_of_vertices);
    return diameter;
}


//
// find the shortest path from a given word to another given word (to be done)
//

static void path_finder(hash_table_t *hash_table,const char *from_word,const char *to_word)
{
  hash_table_node_t *from_node,*to_node, *fromRep, *toRep, **list_of_vertices, *node;
  int final_index;
  unsigned int i;

  // find the nodes for the given words
  from_node = find_word(hash_table,from_word,0);
  to_node = find_word(hash_table,to_word,0);

  // find the representative nodes for the connected components containing the given nodes
  fromRep = find_representative(from_node);
  toRep = find_representative(to_node);

  // check if the words exist in the hash table
  if(from_node == NULL || to_node == NULL)
  {
    printf("One of the words does not exist in the hash table.\n");
    return;
  }

  // check if the words belong to the same connected component
  if(fromRep != toRep)
  {
    printf("The words '%s' and '%s' do not belong to the same connected component.\n",from_word,to_word);
    return;
  }

  // allocate memory for the list of vertices
  list_of_vertices = malloc(fromRep->number_of_vertices * sizeof(hash_table_node_t *));
  
  if(list_of_vertices == NULL)
  {
    fprintf(stderr,"path_finder: unable to allocate memory for the list of vertices\n");
    exit(1);
  }

  // find the shortest path
  final_index = breadth_first_search(hash_table->number_of_entries,list_of_vertices,to_node,from_node);
  node = list_of_vertices[final_index - 1];

  // print the shortest path
  printf("\nThe shortest path from '%s' to '%s' is:\n",from_word,to_word);
  
  while (node != NULL)
  {
    printf("%s \n", node->word);
    node = node ->  previous;
  }
  // free memory
  free(list_of_vertices);
}


//
// some graph information (optional)
//

static void graph_info(hash_table_t *hash_table)
{
  int nrRepresentatives = 0;
  int smallest_diameter = 1000000; //d
  int largest_diameter = 0; //d
  hash_table_t **representatives = (hash_table_t **)malloc(hash_table->number_of_entries * sizeof(hash_table_t));

  // Find the representatives of each connected component
  for (int i = 0; i < hash_table->hash_table_size; i++)
  {
    for (hash_table_node_t *vertex = hash_table->heads[i]; vertex != NULL; vertex = vertex->next)
    {
      // Find the representative of the connected component
      hash_table_node_t *representative = find_representative(vertex);

      // Add the representative to the array if it has not already been added
      if (!representative->visited)
      {
        representatives[nrRepresentatives++] = representative;
        representative->visited = 1;

        // Find the diameter of the connected component
        int diam = connected_component_diameter(representative)-1;
        if (diam > largest_diameter)
        {
          largest_diameter = diam;
        }
        if (diam < smallest_diameter)
        {
          smallest_diameter = diam;
        }
      }
    }
  }

  // Reset the visited status of all vertices
  for (int i = 0; i < hash_table->hash_table_size; i++)
  {
    for (hash_table_node_t *vertex = hash_table->heads[i]; vertex != NULL; vertex = vertex->next)
    {
      vertex->visited = 0;
    }
  }

  // Find the largest connected component
  int largestComponent = 0;
  for (int i = 0; i < nrRepresentatives; i++)
  {
    int componentSize = 0;
    for (int j = 0; j < hash_table->hash_table_size; j++)
    {
      for (hash_table_node_t *vertex = hash_table->heads[j]; vertex != NULL; vertex = vertex->next)
      {
        if (find_representative(vertex) == representatives[i])
        {
          componentSize++;
        }
      }
    }
    //largest_diameter=connected_component_diameter(hash_table);

    if (componentSize > largestComponent)
    {
      largestComponent = componentSize;
    }
  }
  
  //display the number of edges
  printf("\nNumber of edges: %d \n", hash_table->number_of_edges);
  //display the numver of vertices
  printf("Number of vertices: %d \n", hash_table->number_of_entries);
  //display the number of connected components
  printf("Number of connected components: %d \n", nrRepresentatives);
  //display the number of vertices in the largest connected component
  printf("Number of vertices in the largest connected component: %d \n", largestComponent);
  //display the average number of vertices in a connected component
  printf("Average number of vertices in a connected component: %f \n", (float)hash_table->number_of_entries / nrRepresentatives);
  //display the average number of edges in a connected component
  printf("Average number of edges in a connected component: %f \n", (float)hash_table->number_of_edges / nrRepresentatives);
  //display the average degree of a vertex
  printf("Average degree of a vertex: %f \n", (float)hash_table->number_of_edges / hash_table->number_of_entries);
  //display largest diameter of the graph
  printf("Largest diameter of the graph: %d \n", largest_diameter);
  //display the smallest diameter of the graph
  printf("Smallest diameter of the graph: %d \n", smallest_diameter);
  

  free(representatives);
}

static void hash_table_info(hash_table_t *hash_table){

  int number_of_collisions = 0, number_of_empty_lists = 0, max = 0, min = 100000, number_of_lists = 0;
  for (int i = 0; i < hash_table->hash_table_size; i++){
    int length = 0;
    hash_table_node_t *node = hash_table->heads[i];
    while (node != NULL){
      length++;
      node = node->next;
    }
    if (length > max){
      max = length;
    }
    if (length < min && length != 0){
      min = length;
    }
    if (length == 0){
      number_of_empty_lists++;
    }
  }

  printf("\nNumber of entries: %d\n", hash_table->number_of_entries);
  printf("Number of empty list: %d\n", number_of_empty_lists);
  printf("Load factor: %f\n", (float)hash_table->number_of_entries / (float)hash_table->hash_table_size);
  printf("Max length of linked lists: %d\n", max);
  printf("Min length of linked lists: %d\n", min);

}

//
// main program
//

int main(int argc,char **argv)
{
  char word[100],from[100],to[100];
  hash_table_t *hash_table;
  hash_table_node_t *node;
  unsigned int i;
  int command;
  FILE *fp;

  // initialize hash table
  hash_table = hash_table_create();
  // read words
  fp = fopen((argc < 2) ? "wordlist-four-letters.txt" : argv[1],"rb");
  if(fp == NULL)
  {
    fprintf(stderr,"main: unable to open the words file\n");
    exit(1);
  }
  while(fscanf(fp,"%99s",word) == 1)
    (void)find_word(hash_table,word,1);
  fclose(fp);
  // find all similar words
  for(i = 0u;i < hash_table->hash_table_size;i++)
    for(node = hash_table->heads[i];node != NULL;node = node->next)
      similar_words(hash_table,node);
  //graph_info(hash_table);
  //hash_table_print(hash_table);
  // ask what to do
  for(;;)
  {
    fprintf(stderr,"\nYour wish is my command:\n");
    fprintf(stderr,"  1 WORD       (list the connected component WORD belongs to)\n");
    fprintf(stderr,"  2 FROM TO    (list the shortest path from FROM to TO)\n");
    fprintf(stderr,"  3 GRAPH INFO (graph_info)\n");
    fprintf(stderr,"  4 HASH TABLE INFO (hash_table_info)\n");
    fprintf(stderr,"  5            (terminate)\n");
    fprintf(stderr,"> ");
    if(scanf("%99s",word) != 1)
      break;
    command = atoi(word);
    if(command == 1)
    {
      if(scanf("%99s",word) != 1)
        break;
      list_connected_component(hash_table,word);
    }
    else if(command == 2)
    {
      if(scanf("%99s",from) != 1)
        break;
      if(scanf("%99s",to) != 1)
        break;
      path_finder(hash_table,from,to);
    }
    else if(command == 3){
      graph_info(hash_table);
    }
      
    else if (command == 4)
    {
      hash_table_info(hash_table);

    } else if (command == 5){
      break;
    }
  }
  // clean up
  hash_table_free(hash_table);
  return 0;
}