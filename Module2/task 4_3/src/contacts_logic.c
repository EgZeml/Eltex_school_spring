#include "contacts_logic.h"
#include "contacts_types.h"
#include <stdlib.h>
#include <string.h>

// Сравнение контактов по фамилии
// Возвращает: -1, если a < b,
//              0, если равны,
//              1, если a > b.
int compare_contacts(Contact* a, Contact* b) {
    int cmp = strcasecmp(a->name.last_name, b->name.last_name);
    if (cmp == 0) {
        if (a->id < b->id)
            return -1;
        else if (a->id > b->id)
            return 1;
        else
            return 0;
    }
    return cmp;
}


// Вставка нового контакта в бинарное дерево
Tree_node* insert(Tree_node* root, Contact* new_contact) {
    // Вставка нового контакта
    if (root == NULL) {
        Tree_node* new_node = (Tree_node*)malloc(sizeof(Tree_node));
        if (new_node == NULL) {
            return NULL;
        }
        new_node->data = *new_contact;
        new_node->left = new_node->right = NULL;
        return new_node;
    }
    
    // Сравнение текущего узла с новым контактом
    if (compare_contacts(&root->data, new_contact) < 0) {
        root->right = insert(root->right, new_contact);
    } else {
        root->left = insert(root->left, new_contact);
    }
    return root;
}

// Добавление нового контакта в книгу контактов
int add_logic(Contacts_book* book, Contact* new_contact) {
    new_contact->id = book->next_id++;
    book->root = insert(book->root, new_contact);
    if (book->root == NULL) {
        return -1;
    }
    book->count++;
    return 0;
}

// Поиск контакта в дереве по id
Tree_node* search (Tree_node* root, int id) {
    if (!root) {
        return NULL;
    } 

    if (root->data.id == id) {
        return root;
    }

    Tree_node* found_in_left = search(root->left, id);
    if (found_in_left != NULL) {
        return found_in_left;
    }

    return search(root->right, id);
}


// Поиск минимального узла (самого левого в правом дереве) в дереве
Tree_node* find_min(Tree_node* node) {
    if (node == NULL) {
        return NULL;
    }
    while (node->left != NULL) {
        node = node->left;
    }
    return node;
}

// Удаление узла с заданным id из дерева
Tree_node* delete_node(Tree_node* root, int id) {
    if (root == NULL) {
        return NULL;
    }
    
    if (root->data.id == id) {
        // Нет левого поддерева: возвращаем правое
        if (root->left == NULL) {
            Tree_node* temp = root->right;
            free(root);
            return temp;
        }
        // Нет правого поддерева: возвращаем левое
        else if (root->right == NULL) {
            Tree_node* temp = root->left;
            free(root);
            return temp;
        }
        // Есть оба поддерева: находим минимальный узел в правом поддереве
        else {
            Tree_node* temp = find_min(root->right);
            root->data = temp->data;
            root->right = delete_node(root->right, temp->data.id);
        }
        return root;
    }
    
    root->left = delete_node(root->left, id);
    root->right = delete_node(root->right, id);
    return root;
}

// Удаление контакта из книги по id
int delete_logic(Contacts_book* book, int id) {
    if (search(book->root, id) == NULL) {
        return -1;
    }
    book->root = delete_node(book->root, id);
    book->count--;
    return 0;
}

// Редактирование контакта
// Находит контакт по id, удаляет старую запись и вставляет обновленную, сохраняя id
int edit_logic(Contacts_book* book, int id, Contact* updated_contact) {
    Tree_node* node = search(book->root, id);
    if (node == NULL) {
        return -1; // Контакт не найден
    }
    
    // Сохраняем текущий id.
    updated_contact->id = node->data.id;
    
    // Удаляем старую запись.
    book->root = delete_node(book->root, id);
    book->count--;
    
    // Вставляем обновленный контакт.
    book->root = insert(book->root, updated_contact);
    book->count++;
    
    return 0;
}

// Обход дерева и сохранение узлов в массив
void array_inorder(Tree_node* root, Tree_node** array_nodes, int* index) {
    if (root == NULL) {
        return;
    }
    array_inorder(root->left, array_nodes, index);
    array_nodes[*index] = root;
    (*index)++;
    array_inorder(root->right, array_nodes, index);
}

// Балансировка дерева: сборка массива в дерево
Tree_node* build_balanced_tree(Tree_node** array_nodes, int start, int end) {
    if (start > end) {
        return NULL;
    }

    int mid = (start + end) / 2;

    Tree_node* root = array_nodes[mid];

    root->left = build_balanced_tree(array_nodes, start, mid - 1);
    root->right = build_balanced_tree(array_nodes, mid + 1, end);

    return root;

}

// Балансировка дерева: превратить в массив с собрать в дерево снова
void balance_tree(Contacts_book* book) {
    if (book->count < 1) {
        return;
    }

    Tree_node** array_nodes = (Tree_node**)(malloc(sizeof(Tree_node*) * book->count));
    if (array_nodes == NULL) {
        return;
    }

    int index = 0;

    array_inorder(book->root, array_nodes, &index);

    book->root = build_balanced_tree(array_nodes, 0, book->count - 1);
    free(array_nodes);
}