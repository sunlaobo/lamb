<?php

/*
 * The Account Controller
 * Link http://github.com/typefo/lamb
 * Copyright (C) typefo <typefo@qq.com>
 */

use Tool\Filter;

class AccountController extends Yaf\Controller_Abstract {
    public function indexAction() {
        $company = new CompanyModel();
        $list = array();
        foreach ($company->getAll() as $key => $val) {
            $list[$val['id']] = $val;
        }
        $account = new AccountModel();
        $this->getView()->assign('companys', $list);
        $this->getView()->assign('accounts', $account->getAll());
        return true;
    }

    public function createAction() {
        $request = $this->getRequest();

        if ($request->isPost()) {
            $account = new AccountModel();
            $account->create($request->getPost());
            $url = 'http://' . $_SERVER['SERVER_ADDR'] . ':' . $_SERVER['SERVER_PORT'] . '/account';
            $response = $this->getResponse();
            $response->setRedirect($url);
            $response->response();
            return false;
        }

        $company = new CompanyModel();
        $list = $company->getAll();

        $this->getView()->assign('companys', $list);
        return true;
    }

    public function editAction() {
        $request = $this->getRequest();
        $account = new AccountModel();

        if ($request->isPost()) {
            $account->change($request->getPost('id'), $request->getPost());
            $url = 'http://' . $_SERVER['SERVER_ADDR'] . ':' . $_SERVER['SERVER_PORT'] . '/account';
            $response = $this->getResponse();
            $response->setRedirect($url);
            $response->response();
            return false;
        }

        $company = new CompanyModel();
        $this->getView()->assign('companys', $company->getAll());
        $this->getView()->assign('account', $account->get($request->getQuery('id')));
        return true;
    }


    public function deleteAction() {
        $request = $this->getRequest();
        $account = new AccountModel();
        $success = $account->delete($request->getQuery('id'));
        $response['status'] = $success ? 200 : 400;
        $response['message'] = $success ? 'success' : 'failed';
        header('Content-type: application/json');
        echo json_encode($response);
        return false;
    }
}
