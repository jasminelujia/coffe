/*
 * This file is part of COFFE
 * Copyright (C) 2018 Goran Jelic-Cizmek
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful, but
 * WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the GNU
 * General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program. If not, see <http://www.gnu.org/licenses/>.
*/

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>
#include <stdarg.h>
#include <gsl/gsl_version.h>
#include <gsl/gsl_interp.h>

#include "common.h"
#include "errors.h"


/**
    another malloc function, checks if malloc succeded
**/

void *coffe_malloc(size_t len)
{
    void *values = malloc(len);
    if (values == NULL){
        print_error(PROG_ALLOC_ERROR);
        exit(EXIT_FAILURE);
    }
    return values;
}


/**
    gets the current time
**/

char *coffe_get_time(void)
{
    char *timestamp = (char *)malloc(sizeof(char)*30);
    time_t ltime;
    ltime = time(NULL);
    struct tm *tm;
    tm = localtime(&ltime);

    sprintf(
        timestamp,"%02d-%02d-%02d-%02d-%02d",
        tm->tm_mon + 1, tm->tm_mday, tm->tm_hour, tm->tm_min, tm->tm_sec
    );
    return timestamp;
}


/**
    reads file <filename> into array <values>
    of length <len>
**/

int read_1col(
    char *filename,
    double **values,
    size_t *len
)
{
    int error = 0;
    FILE *data = fopen(filename, "r");
    if (data == NULL){
        print_error_verbose(PROG_OPEN_ERROR, filename);
        return EXIT_FAILURE;
    }
    size_t n = 0;
    char c, temp_string[COFFE_MAX_STRLEN];

    while ((c=fgetc(data)) != EOF){
        if (c == '\n') ++n;
    }

    error = fseek(data, 0, SEEK_SET);
    if (error){
        print_error_verbose(PROG_POS_ERROR, filename);
        return EXIT_FAILURE;
    }

    size_t counter = 0;
    for (size_t i = 0; i<n; ++i){
        fgets(temp_string, COFFE_MAX_STRLEN, data);
        if (temp_string[0] != '#') ++counter;
    }
    *len = counter;

    error = fseek(data, 0, SEEK_SET);
    if (error){
        print_error_verbose(PROG_POS_ERROR, filename);
        return EXIT_FAILURE;
    }

    *values = (double *)coffe_malloc(sizeof(double)*counter);
    counter = 0;

    for (size_t i = 0; i<n; ++i){
        fgets(temp_string, COFFE_MAX_STRLEN, data);
        if (temp_string[0] != '#'){
            *(*values + counter) = atof(temp_string);
            ++counter;
        }
    }

    error = fclose(data);
    if (error){
        print_error_verbose(PROG_CLOSE_ERROR, filename);
        return EXIT_FAILURE;
    }
    return EXIT_SUCCESS;
}


/**
    reads file <filename> into arrays <values1>
    and <values2> of length <len>
**/

int read_2col(
    char *filename,
    double **values1,
    double **values2,
    size_t *len
)
{
    int error = 0;
    FILE *data = fopen(filename, "r");
    if (data == NULL){
        print_error_verbose(PROG_OPEN_ERROR, filename);
        return EXIT_FAILURE;
    }
    size_t n = 0;
    char c, temp_string[COFFE_MAX_STRLEN];
    char *temp_token;

    while ((c=fgetc(data)) != EOF){
        if (c == '\n') ++n;
    }

    error = fseek(data, 0, SEEK_SET);
    if (error){
        print_error_verbose(PROG_POS_ERROR, filename);
        return EXIT_FAILURE;
    }

    size_t counter = 0;
    for (size_t i = 0; i<n; ++i){
        fgets(temp_string, COFFE_MAX_STRLEN, data);
        if (temp_string[0] != '#') ++counter;
    }
    *len = counter;

    error = fseek(data, 0, SEEK_SET);
    if (error){
        print_error_verbose(PROG_POS_ERROR, filename);
        return EXIT_FAILURE;
    }

    *values1 = (double *)coffe_malloc(sizeof(double)*counter);
    *values2 = (double *)coffe_malloc(sizeof(double)*counter);
    counter = 0;

    for (size_t i = 0; i<n; ++i){
        fgets(temp_string, COFFE_MAX_STRLEN, data);
        if (temp_string[0] != '#'){
            temp_token = strtok(temp_string, ",\t: ");
            *(*values1 + counter) = atof(temp_token);
            temp_token = strtok(NULL, ",\t: ");
            *(*values2 + counter) = atof(temp_token);
            ++counter;
        }
    }

    error = fclose(data);
    if (error){
        print_error_verbose(PROG_CLOSE_ERROR, filename);
        return EXIT_FAILURE;
    }
    return EXIT_SUCCESS;
}


/**
    writes array <values> of length <len>
    into file <filename> with header <header>
**/

int write_1col(
    char *filename,
    double *values,
    size_t len,
    const char *header
)
{
    int error = 0;
    FILE *data = fopen(filename, "w");
    if (data == NULL){
        print_error_verbose(PROG_OPEN_ERROR, filename);
        return EXIT_FAILURE;
    }

    if (header != NULL)
        fprintf(data, "%s", header);

    for (size_t i = 0; i<len; ++i)
        fprintf(data, "%.15e\n", values[i]);

    error = fclose(data);
    if (error){
        print_error_verbose(PROG_CLOSE_ERROR, filename);
        return EXIT_FAILURE;
    }
    return EXIT_SUCCESS;
}


/**
    writes arrays <values1> and <values2> of length <len>
    into file <filename> with header <header>
    using separator(s) <sep>
**/

int write_2col(
    char *filename,
    double *values1,
    double *values2,
    size_t len,
    const char *header,
    const char *sep
)
{
    int error = 0;
    FILE *data = fopen(filename, "w");
    if (data == NULL){
        print_error_verbose(PROG_OPEN_ERROR, filename);
        return EXIT_FAILURE;
    }

    if (header != NULL)
        fprintf(data, "%s", header);

    for (size_t i = 0; i<len; ++i)
        fprintf(data, "%.15e%s%.15e\n", values1[i], sep, values2[i]);

    error = fclose(data);
    if (error){
        print_error_verbose(PROG_CLOSE_ERROR, filename);
        return EXIT_FAILURE;
    }
    return EXIT_SUCCESS;
}


/**
    writes an arbitrary number of columns into file <filename>,
    each column having length <len>,
    with header <header> and using separator <sep>
    NOTE: C can't figure out by itself the number of
    arguments, so the first parameter is the number of columns
**/

int write_ncol(
    size_t ncolumns,
    char *filename,
    size_t len,
    const char *header,
    const char *sep,
    double *values,
    ...
)
{
    va_list args;
    FILE *data = fopen(filename, "w");
    if (data == NULL){
        print_error_verbose(PROG_OPEN_ERROR, filename);
        return EXIT_FAILURE;
    }
    if (header != NULL) fprintf(data, "%s", header);

    va_start(args, values);
    double **all_values = (double **)coffe_malloc(sizeof(double *)*ncolumns);
    all_values[0] = values;

    for (size_t i = 1; i<ncolumns; ++i){
        values = va_arg(args, double *);
        all_values[i] = values;
    }
    va_end(args);

    for (size_t i = 0; i<len; ++i){
        if (all_values[i] != NULL){
            for (size_t j = 0; j<ncolumns; ++j){
                if (j<ncolumns - 1)
                    fprintf(data, "%.10e%s", all_values[j][i], sep);
                else
                    fprintf(data, "%.10e%s\n", all_values[j][i], sep);
            }
        }
    }
    fclose(data);
    for (size_t i = 0; i<ncolumns; ++i){
        all_values[i] = NULL;
    }
    free(all_values);
    return EXIT_SUCCESS;
}


/**
    modification of the above, but without the counter;
    last argument MUST be NULL for the function to work properly!
**/

int write_ncol_null(
    char *filename,
    size_t len,
    const char *header,
    const char *sep,
    double *values,
    ...
)
{
    va_list args;
    FILE *data = fopen(filename, "w");
    if (data == NULL){
        print_error_verbose(PROG_OPEN_ERROR, filename);
        return EXIT_FAILURE;
    }
    if (header != NULL) fprintf(data, "%s", header);

    va_start(args, values);
    size_t counter = 0;
    double **all_values = (double **)coffe_malloc(sizeof(double *)*COFFE_MAX_ALLOCABLE);
    all_values[0] = values;

    do{
        values = va_arg(args, double *);
        ++counter;
        all_values[counter] = values;
    } while (values != NULL);
    va_end(args);

    for (size_t i = 0; i<len; ++i){
        for (size_t j = 0; j<counter; ++j){
            if (j<counter - 1)
                fprintf(data, "%.10e%s", all_values[j][i], sep);
            else
                fprintf(data, "%.10e%s\n", all_values[j][i], sep);
        }
    }
    fclose(data);
    for (size_t i = 0; i<counter; ++i){
        all_values[i] = NULL;
    }
    free(all_values);
    return EXIT_SUCCESS;
}



/**
    writes a <len1>x<len2> matrix <values> into file <filename>
**/

int write_matrix(
    char *filename,
    double **values,
    size_t len1,
    size_t len2,
    const char *header,
    const char *sep
)
{
    int error = 0;
    FILE *data = fopen(filename, "w");
    if (data == NULL){
        print_error(PROG_OPEN_ERROR);
        return EXIT_FAILURE;
    }

    if (header != NULL){
        fprintf(data, "%s\n", header);
    }

    for (size_t i = 0; i<len1; ++i){
        for (size_t j = 0; j<len2; ++j){
            if (j != len2 - 1)
                fprintf(data, "%10e%s", values[i][j], sep);
            else
                fprintf(data, "\n");
        }
    }

    error = fclose(data);
    if (error){
        print_error(PROG_CLOSE_ERROR);
        return EXIT_FAILURE;
    }
    return EXIT_SUCCESS;
}


/**
    allocates an <len1>x<len2> matrix
    and stores it into <values>
**/

int alloc_double_matrix(
    double ***values,
    size_t len1,
    size_t len2
)
{
    *values = (double **)coffe_malloc(sizeof(double *)*len1);
    for (size_t i = 0; i<len1; ++i){
        (*values)[i] = (double *)coffe_malloc(sizeof(double)*len2);
    }
    return EXIT_SUCCESS;
}


int free_double_matrix(
    double ***values,
    size_t len
)
{
    *values = (double **)coffe_malloc(sizeof(double *)*len);
    for (size_t i = 0; i<len; ++i){
        free((*values)[i]);
    }
    free(*values);
    return EXIT_SUCCESS;
}


int copy_matrix_array(
    double **destination,
    double **source,
    size_t rows,
    size_t columns,
    size_t index,
    char *type
)
{
    if (strcmp(type, "row") == 0){
        *destination = (double *)coffe_malloc(sizeof(double)*columns);
        if (index >= rows){
            print_error(PROG_FAIL);
            exit(EXIT_FAILURE);
        }
        for (size_t i = 0; i<columns; ++i)
            (*destination)[i] = source[index][i];
    }
    else if (strcmp(type, "column") == 0){
        *destination = (double *)coffe_malloc(sizeof(double)*rows);
        if (index >= columns){
            print_error(PROG_FAIL);
            exit(EXIT_FAILURE);
        }
        for (size_t i = 0; i<rows; ++i)
            (*destination)[i] = source[i][index];
    }

    return EXIT_SUCCESS;
}


int init_spline(
    struct coffe_interpolation *interp,
    double *xi,
    double *yi,
    size_t bins,
    int interpolation_type
)
{
    if (bins <= 0){
        print_error(PROG_VALUE_ERROR);
        exit(EXIT_FAILURE);
    }
    const gsl_interp_type *T;
    switch(interpolation_type){
        case 1:
            T = gsl_interp_linear;
            break;
        case 2:
            T = gsl_interp_polynomial;
            break;
        case 3:
            T = gsl_interp_cspline;
            break;
        case 4:
            T = gsl_interp_cspline_periodic;
            break;
        case 5:
            T = gsl_interp_akima;
            break;
        case 6:
            T = gsl_interp_akima_periodic;
            break;
        case 7:{
#if GSL_MAJOR_VERSION > 1
                T = gsl_interp_steffen;
                break;
#else
                fprintf(stderr,
                    "Interpolation type \"Steffen\" requires GSL 2.1 or above!\n");
                exit(EXIT_SUCCESS);
#endif
        }
        default:
            T = gsl_interp_akima;
            break;
    }
    interp->spline
        = gsl_spline_alloc(T, bins);
    interp->accel
        = gsl_interp_accel_alloc();
    gsl_spline_init(interp->spline, xi, yi, bins);
    return EXIT_SUCCESS;
}


int free_spline(
    struct coffe_interpolation *interp
)
{
    gsl_spline_free(interp->spline);
    gsl_interp_accel_free(interp->accel);
    if (interp->spline != NULL) interp->spline = NULL;
    if (interp->accel != NULL) interp->accel = NULL;
    return EXIT_SUCCESS;
}


int coffe_compare_ascending(
    const void *a,
    const void *b
)
{
    if (*(double *)a > *(double *)b) return 1;
    else if (*(double *)a < *(double *)b) return -1;
    else return 0;
}

int coffe_compare_descending(
    const void *a,
    const void *b
)
{
    if (*(double *)a > *(double *)b) return -1;
    else if (*(double *)a < *(double *)b) return 1;
    else return 0;
}



/**
    function describing w(z)
    for now only w(z) = w0 + wa*(1 - a),
    but others straightforward to implement
**/

double common_wfunction(
    struct coffe_parameters_t *par,
    double z
)
{
    return par->w0 + par->wa*z/(1 + z);
}

double interp_spline(
    struct coffe_interpolation *interp,
    double value
)
{
    return gsl_spline_eval(interp->spline, value, interp->accel);
}

